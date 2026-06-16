#include "bvh3dmodel.h"

#include "bvhbonelistmodel.h"
#include "bvhfile.h"
#include "bvhjointlistmodel.h"
#include "bvhnode.h"
#include "pose_utils.h"

#include <QHash>
#include <QVector4D>
#include <QtMath>
#include <algorithm>

namespace {

QColor boneColorFromTone(const QColor& jointColor, int tone)
{
    if (tone == 0) {
        return jointColor;
    }

    const int magnitude = qAbs(tone);
    const int qtFactor = 100 + magnitude;
    if (tone > 0) {
        return jointColor.lighter(qtFactor);
    }
    return jointColor.darker(qtFactor);
}

} // namespace

QColor Bvh3DModel::colorFromTone(const QColor& jointColor, int tone)
{
    return boneColorFromTone(jointColor, tone);
}

int Bvh3DModel::defaultBoneTone()
{
    return DefaultBoneTone;
}

Bvh3DModel::Bvh3DModel(QObject* parent)
    : QObject(parent)
    , m_jointModel(new BvhJointListModel(this))
    , m_boneModel(new BvhBoneListModel(this))
{
    m_jointModel->setJoints(&m_joints);
    m_boneModel->setBones(&m_bones);
}

Bvh3DModel::Bvh3DModel(std::shared_ptr<BvhFile> bvhFile, QObject* parent)
    : Bvh3DModel(parent)
{
    attach(std::move(bvhFile));
}

Bvh3DModel::~Bvh3DModel() = default;

bool Bvh3DModel::attach(std::shared_ptr<BvhFile> bvhFile)
{
    if (!bvhFile || !bvhFile->isValid()) {
        const QString reason = !bvhFile ? QStringLiteral("BvhFile is null")
                                        : QStringLiteral("BvhFile is invalid");
        reset();
        emit attachFailed(reason);
        return false;
    }

    m_bvhFile = std::move(bvhFile);
    buildSkeletonMetadata();
    applyInitialPose();

    emit bvhFileChanged();
    return true;
}

bool Bvh3DModel::isValid() const
{
    return m_bvhFile && m_bvhFile->isValid();
}

int Bvh3DModel::frameCount() const
{
    return m_bvhFile ? m_bvhFile->getFrameCount() : 0;
}

double Bvh3DModel::frameTime() const
{
    return m_bvhFile ? m_bvhFile->getFrameTime() : 0.0;
}

void Bvh3DModel::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    m_visible = visible;
    emit visibleChanged();
}

void Bvh3DModel::setJointColor(const QColor& color)
{
    if (m_jointColor == color) {
        return;
    }

    m_jointColor = color;
    emit jointColorChanged();

    if (m_boneColorMode == BoneColorMode::SameAsJoint || m_boneColorMode == BoneColorMode::ToneOffset) {
        applyEffectiveBoneColor();
    }
}

void Bvh3DModel::setBoneColorMode(BoneColorMode mode)
{
    if (m_boneColorMode == mode) {
        return;
    }
    m_boneColorMode = mode;
    emit boneColorModeChanged();
    applyEffectiveBoneColor();
}

void Bvh3DModel::setBoneTone(int tone)
{
    tone = qBound(-100, tone, 100);
    if (m_boneTone == tone) {
        return;
    }
    m_boneTone = tone;
    emit boneToneChanged();

    if (m_boneColorMode == BoneColorMode::ToneOffset) {
        applyEffectiveBoneColor();
    }
}

void Bvh3DModel::setCustomBoneColor(const QColor& color)
{
    if (m_customBoneColor == color) {
        return;
    }
    m_customBoneColor = color;
    emit customBoneColorChanged();

    if (m_boneColorMode == BoneColorMode::Custom) {
        applyEffectiveBoneColor();
    }
}

QColor Bvh3DModel::effectiveBoneColor() const
{
    switch (m_boneColorMode) {
    case BoneColorMode::SameAsJoint:
        return m_jointColor;
    case BoneColorMode::ToneOffset:
        return boneColorFromTone(m_jointColor, m_boneTone);
    case BoneColorMode::Custom:
        return m_customBoneColor;
    }
    return m_jointColor;
}

void Bvh3DModel::applyEffectiveBoneColor()
{
    const QColor effective = effectiveBoneColor();
    if (m_boneColor == effective) {
        return;
    }
    m_boneColor = effective;
    emit boneColorChanged();
}

void Bvh3DModel::setDisplayName(const QString& name)
{
    if (m_displayName == name) {
        return;
    }
    m_displayName = name;
    emit displayNameChanged();
}

void Bvh3DModel::setFrame(int frameIndex)
{
    if (!isValid()) {
        return;
    }

    const int count = frameCount();
    if (count <= 0) {
        applyInitialPose();
        return;
    }

    const int clampedFrame = std::clamp(frameIndex, 0, count - 1);
    m_bvhFile->setCurrentFrame(clampedFrame);

    if (m_currentFrame != clampedFrame) {
        m_currentFrame = clampedFrame;
        emit currentFrameChanged();
    }

    updatePoseFromBvhFile(true);
}

void Bvh3DModel::setPoseAtTime(double seconds)
{
    if (!isValid()) {
        return;
    }

    const double frameTime = m_bvhFile->getFrameTime();
    if (frameTime <= 0.0 || frameCount() <= 0) {
        applyInitialPose();
        return;
    }

    const double frameReal = seconds / frameTime;
    PoseUtils::applyInterpolatedFrame(m_bvhFile.get(), frameReal);

    const int derivedFrame = static_cast<int>(std::round(frameReal));
    const int clampedFrame = std::clamp(derivedFrame, 0, std::max(0, frameCount() - 1));
    if (m_currentFrame != clampedFrame) {
        m_currentFrame = clampedFrame;
        emit currentFrameChanged();
    }

    updatePoseFromBvhFile(false);
}

void Bvh3DModel::buildSkeletonMetadata()
{
    m_joints.clear();
    m_bones.clear();

    if (!m_bvhFile) {
        m_jointModel->setJoints(&m_joints);
        m_boneModel->setBones(&m_bones);
        return;
    }

    QHash<BvhNode*, int> nodeToJointIndex;
    const QList<BvhNode*>& nodes = m_bvhFile->getAllNodes();

    for (int nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex) {
        BvhNode* node = nodes.at(nodeIndex);

        Bvh3DJoint joint;
        joint.index = m_joints.size();
        joint.bvhNodeIndex = nodeIndex;
        joint.name = node->getName();
        joint.isEndSite = false;
        m_joints.append(joint);
        nodeToJointIndex.insert(node, joint.index);

        if (node->hasEndSiteInfo()) {
            Bvh3DJoint endSiteJoint;
            endSiteJoint.index = m_joints.size();
            endSiteJoint.bvhNodeIndex = nodeIndex;
            endSiteJoint.name = node->getName() + QStringLiteral("_EndSite");
            endSiteJoint.isEndSite = true;
            m_joints.append(endSiteJoint);
        }
    }

    for (int nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex) {
        BvhNode* node = nodes.at(nodeIndex);
        BvhNode* parent = node->getParent();
        if (!parent) {
            continue;
        }

        Bvh3DBone bone;
        bone.index = m_bones.size();
        bone.parentJointIndex = nodeToJointIndex.value(parent);
        bone.childJointIndex = nodeToJointIndex.value(node);
        m_bones.append(bone);

        if (node->hasEndSiteInfo()) {
            Bvh3DBone endSiteBone;
            endSiteBone.index = m_bones.size();
            endSiteBone.parentJointIndex = nodeToJointIndex.value(node);
            endSiteBone.childJointIndex = nodeToJointIndex.value(node) + 1;
            m_bones.append(endSiteBone);
        }
    }

    m_jointModel->setJoints(&m_joints);
    m_boneModel->setBones(&m_bones);
}

void Bvh3DModel::updatePoseFromBvhFile(bool immediateNotify)
{
    if (!isValid()) {
        return;
    }

    const QList<BvhNode*>& nodes = m_bvhFile->getAllNodes();

    for (int i = 0; i < m_joints.size(); ++i) {
        Bvh3DJoint& joint = m_joints[i];
        if (joint.isEndSite) {
            BvhNode* parentNode = nodes.at(joint.bvhNodeIndex);
            const QVector3D localEndSite = parentNode->getEndSiteOffset();
            const QVector4D world = parentNode->getWorldTransform() * QVector4D(localEndSite, 1.0f);
            joint.position = world.toVector3D();
        } else {
            BvhNode* node = nodes.at(joint.bvhNodeIndex);
            joint.position = node->getWorldPosition();
        }
    }

    for (int i = 0; i < m_bones.size(); ++i) {
        Bvh3DBone& bone = m_bones[i];
        const QVector3D parentPosition = m_joints.at(bone.parentJointIndex).position;
        const QVector3D childPosition = m_joints.at(bone.childJointIndex).position;
        const PoseUtils::BoneTransform transform =
            PoseUtils::computeBoneTransform(parentPosition, childPosition);
        bone.position = transform.position;
        bone.rotation = transform.rotation;
        bone.length = transform.length;
    }

    if (immediateNotify) {
        notifyPoseChanged();
        return;
    }

    if (!m_poseRefreshScheduled) {
        m_poseRefreshScheduled = true;
        QMetaObject::invokeMethod(this, &Bvh3DModel::flushPoseRefresh, Qt::QueuedConnection);
    }
}

void Bvh3DModel::notifyPoseChanged()
{
    m_poseRefreshScheduled = false;
    m_jointModel->refreshAll();
    m_boneModel->refreshAll();
    emit poseUpdated();
}

void Bvh3DModel::flushPoseRefresh()
{
    notifyPoseChanged();
}

void Bvh3DModel::applyInitialPose()
{
    if (!isValid()) {
        return;
    }

    if (frameCount() > 0) {
        setFrame(0);
        return;
    }

    m_currentFrame = -1;
    emit currentFrameChanged();
    updatePoseFromBvhFile(true);
}

void Bvh3DModel::reset()
{
    m_bvhFile.reset();
    m_joints.clear();
    m_bones.clear();
    m_currentFrame = -1;

    m_jointModel->setJoints(&m_joints);
    m_boneModel->setBones(&m_bones);

    emit bvhFileChanged();
    emit currentFrameChanged();
}
