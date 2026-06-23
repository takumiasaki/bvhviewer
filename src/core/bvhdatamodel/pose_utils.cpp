#include "pose_utils.h"

#include "bvhfile.h"
#include "bvhnode.h"
#include "channel.h"

#include <QtMath>
#include <algorithm>

namespace PoseUtils {

namespace {

constexpr float kMinBoneLength = 1e-4f;
constexpr float kDirectionEpsilon = 1e-6f;

bool isRotationChannel(Channel::Type type)
{
    return type == Channel::Xrotation
        || type == Channel::Yrotation
        || type == Channel::Zrotation;
}

double interpolateChannelValue(Channel::Type type, double v0, double v1, double alpha)
{
    if (isRotationChannel(type)) {
        const double delta = std::fmod(v1 - v0 + 540.0, 360.0) - 180.0;
        return v0 + delta * alpha;
    }
    return v0 + (v1 - v0) * alpha;
}

QVector<Channel::Type> channelTypesForFile(const BvhFile* bvhFile)
{
    const int totalChannels = bvhFile->getTotalChannels();
    QVector<Channel::Type> types(totalChannels, Channel::Unknown);

    for (BvhNode* node : bvhFile->getAllNodes()) {
        const int start = node->getChannelStartIndex();
        const QList<Channel>& channels = node->getChannels();
        for (int i = 0; i < channels.size(); ++i) {
            if (start + i < types.size()) {
                types[start + i] = channels.at(i).type;
            }
        }
    }

    return types;
}

QVector<double> interpolateFrameData(const BvhFile* bvhFile, double frameReal)
{
    const int frameCount = bvhFile->getFrameCount();
    if (frameCount <= 0) {
        return {};
    }

    const int totalChannels = bvhFile->getTotalChannels();
    if (frameCount == 1 || totalChannels <= 0) {
        return bvhFile->getFrameData(0);
    }

    const double clampedFrame = std::clamp(frameReal, 0.0, static_cast<double>(frameCount - 1));
    const int f0 = static_cast<int>(std::floor(clampedFrame));
    const int f1 = static_cast<int>(std::ceil(clampedFrame));
    const double alpha = clampedFrame - static_cast<double>(f0);

    const QVector<double> frame0 = bvhFile->getFrameData(f0);
    const QVector<double> frame1 = bvhFile->getFrameData(f1);
    if (frame0.isEmpty()) {
        return frame1;
    }
    if (frame1.isEmpty() || f0 == f1) {
        return frame0;
    }

    const QVector<Channel::Type> channelTypes = channelTypesForFile(bvhFile);

    QVector<double> blended(totalChannels);
    for (int i = 0; i < totalChannels; ++i) {
        const double v0 = i < frame0.size() ? frame0[i] : 0.0;
        const double v1 = i < frame1.size() ? frame1[i] : v0;
        const Channel::Type type = i < channelTypes.size() ? channelTypes[i] : Channel::Unknown;
        blended[i] = interpolateChannelValue(type, v0, v1, alpha);
    }
    return blended;
}

} // namespace

PoseBounds computePoseBounds(const QVector<Bvh3DJoint>& joints,
                             float jointRadius,
                             float endSiteRadius,
                             float marginFactor)
{
    PoseBounds bounds;
    if (joints.isEmpty()) {
        return bounds;
    }

    bounds.valid = true;
    bounds.min = joints.first().position;
    bounds.max = joints.first().position;

    for (const Bvh3DJoint& joint : joints) {
        const float radius = joint.isEndSite ? endSiteRadius : jointRadius;
        bounds.min.setX(qMin(bounds.min.x(), joint.position.x() - radius));
        bounds.min.setY(qMin(bounds.min.y(), joint.position.y() - radius));
        bounds.min.setZ(qMin(bounds.min.z(), joint.position.z() - radius));
        bounds.max.setX(qMax(bounds.max.x(), joint.position.x() + radius));
        bounds.max.setY(qMax(bounds.max.y(), joint.position.y() + radius));
        bounds.max.setZ(qMax(bounds.max.z(), joint.position.z() + radius));
    }

    if (marginFactor != 1.0f) {
        const QVector3D center = boundsCenter(bounds);
        const QVector3D halfExtent = (bounds.max - bounds.min) * 0.5f * marginFactor;
        bounds.min = center - halfExtent;
        bounds.max = center + halfExtent;
    }

    return bounds;
}

PoseBounds mergePoseBounds(const PoseBounds& a, const PoseBounds& b)
{
    if (!a.valid) {
        return b;
    }
    if (!b.valid) {
        return a;
    }

    PoseBounds merged;
    merged.valid = true;
    merged.min = QVector3D(qMin(a.min.x(), b.min.x()),
                           qMin(a.min.y(), b.min.y()),
                           qMin(a.min.z(), b.min.z()));
    merged.max = QVector3D(qMax(a.max.x(), b.max.x()),
                           qMax(a.max.y(), b.max.y()),
                           qMax(a.max.z(), b.max.z()));
    return merged;
}

QVector3D boundsCenter(const PoseBounds& bounds)
{
    if (!bounds.valid) {
        return QVector3D();
    }
    return (bounds.min + bounds.max) * 0.5f;
}

float framingRadius(const PoseBounds& bounds)
{
    if (!bounds.valid) {
        return 0.0f;
    }

    const QVector3D extent = bounds.max - bounds.min;
    const float halfWidth = extent.x() * 0.5f;
    const float halfHeight = extent.y() * 0.5f;
    const float halfDepth = extent.z() * 0.5f;
    return std::sqrt(halfWidth * halfWidth + halfHeight * halfHeight + halfDepth * halfDepth);
}

float framingDistance(const PoseBounds& bounds,
                      float fovDegrees,
                      float aspectRatio,
                      float pitchDegrees)
{
    if (!bounds.valid) {
        return 0.0f;
    }

    const QVector3D extent = bounds.max - bounds.min;
    const float halfWidth = extent.x() * 0.5f;
    const float halfHeight = extent.y() * 0.5f;
    const float halfDepth = extent.z() * 0.5f;

    const float safeAspect = aspectRatio > 0.0f ? aspectRatio : 1.0f;
    const float vFovRad = qDegreesToRadians(fovDegrees);
    const float hFovRad = 2.0f * std::atan(std::tan(vFovRad * 0.5f) * static_cast<double>(safeAspect));

    const float sinV = std::sin(vFovRad * 0.5f);
    const float sinH = std::sin(hFovRad * 0.5f);
    if (sinV < 1e-6f || sinH < 1e-6f) {
        return 0.0f;
    }

    const float distV = halfHeight / sinV;
    const float distH = halfWidth / sinH;
    const float distD = halfDepth / sinV;
    float distance = qMax(distV, qMax(distH, distD));

    const float pitchRad = qDegreesToRadians(std::abs(pitchDegrees));
    const float cosPitch = std::cos(pitchRad);
    if (cosPitch > 0.01f) {
        distance /= cosPitch;
    }

    return distance;
}

BoneTransform computeBoneTransform(const QVector3D& parentPosition,
                                   const QVector3D& childPosition)
{
    BoneTransform result;
    const QVector3D delta = childPosition - parentPosition;
    const float distance = delta.length();

    if (distance < kDirectionEpsilon) {
        result.position = parentPosition;
        result.rotation = QQuaternion();
        result.length = kMinBoneLength;
        return result;
    }

    const QVector3D direction = delta / distance;
    result.rotation = QQuaternion::rotationTo(QVector3D(0.0f, 1.0f, 0.0f), direction);
    result.length = distance;
    result.position = parentPosition + direction * (distance * 0.5f);
    return result;
}

void applyChannelValues(BvhFile* bvhFile, const QVector<double>& frameData)
{
    if (!bvhFile) {
        return;
    }

    const QList<BvhNode*>& nodes = bvhFile->getAllNodes();
    for (BvhNode* node : nodes) {
        const int startIdx = node->getChannelStartIndex();
        const int channelCount = node->getChannelCount();

        for (int i = 0; i < channelCount; ++i) {
            if (startIdx + i < frameData.size()) {
                node->setChannelValue(i, frameData[startIdx + i]);
            }
        }

        node->updateLocalTransform();
    }

    for (BvhNode* node : nodes) {
        BvhNode* parent = node->getParent();
        if (parent) {
            node->setWorldTransform(parent->getWorldTransform() * node->getLocalTransform());
        } else {
            node->setWorldTransform(node->getLocalTransform());
        }
    }
}

void applyInterpolatedFrame(BvhFile* bvhFile, double frameReal)
{
    if (!bvhFile || !bvhFile->isValid()) {
        return;
    }

    const QVector<double> frameData = interpolateFrameData(bvhFile, frameReal);
    if (frameData.isEmpty()) {
        return;
    }

    applyChannelValues(bvhFile, frameData);
}

} // namespace PoseUtils
