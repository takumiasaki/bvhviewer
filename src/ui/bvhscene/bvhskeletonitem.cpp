#include "bvhskeletonitem.h"

#include <QFileInfo>

BvhSkeletonItem::BvhSkeletonItem(Bvh3DModel* model,
                                 const QString& sourcePath,
                                 int paletteIndex,
                                 QObject* parent)
    : QObject(parent)
    , m_model(model)
    , m_sourcePath(sourcePath)
    , m_paletteIndex(paletteIndex)
{
    connectModelSignals();
}

BvhJointListModel* BvhSkeletonItem::jointModel() const
{
    return m_model ? m_model->jointModel() : nullptr;
}

BvhBoneListModel* BvhSkeletonItem::boneModel() const
{
    return m_model ? m_model->boneModel() : nullptr;
}

void BvhSkeletonItem::setSceneOffset(const QVector3D& offset)
{
    if (m_sceneOffset == offset) {
        return;
    }
    m_sceneOffset = offset;
    emit sceneOffsetChanged();
}

bool BvhSkeletonItem::visible() const
{
    return m_model ? m_model->visible() : false;
}

void BvhSkeletonItem::setVisible(bool visible)
{
    if (m_model) {
        m_model->setVisible(visible);
    }
}

QColor BvhSkeletonItem::jointColor() const
{
    return m_model ? m_model->jointColor() : QColor(Qt::white);
}

void BvhSkeletonItem::setJointColor(const QColor& color)
{
    if (m_model) {
        m_model->setJointColor(color);
    }
}

QColor BvhSkeletonItem::boneColor() const
{
    return m_model ? m_model->boneColor() : QColor(Qt::white);
}

Bvh3DModel::BoneColorMode BvhSkeletonItem::boneColorMode() const
{
    return m_model ? m_model->boneColorMode() : Bvh3DModel::ToneOffset;
}

void BvhSkeletonItem::setBoneColorMode(Bvh3DModel::BoneColorMode mode)
{
    if (m_model) {
        m_model->setBoneColorMode(mode);
    }
}

int BvhSkeletonItem::boneTone() const
{
    return m_model ? m_model->boneTone() : Bvh3DModel::defaultBoneTone();
}

void BvhSkeletonItem::setBoneTone(int tone)
{
    if (m_model) {
        m_model->setBoneTone(tone);
    }
}

QColor BvhSkeletonItem::customBoneColor() const
{
    return m_model ? m_model->customBoneColor() : QColor(Qt::white);
}

void BvhSkeletonItem::setCustomBoneColor(const QColor& color)
{
    if (m_model) {
        m_model->setCustomBoneColor(color);
    }
}

QString BvhSkeletonItem::displayName() const
{
    return m_model ? m_model->displayName() : QString();
}

QUrl BvhSkeletonItem::source() const
{
    return QUrl::fromLocalFile(m_sourcePath);
}

bool BvhSkeletonItem::isValid() const
{
    return m_model && m_model->isValid();
}

int BvhSkeletonItem::frameCount() const
{
    return m_model ? m_model->frameCount() : 0;
}

double BvhSkeletonItem::frameTime() const
{
    return m_model ? m_model->frameTime() : 0.0;
}

int BvhSkeletonItem::currentFrame() const
{
    return m_model ? m_model->currentFrame() : -1;
}

void BvhSkeletonItem::connectModelSignals()
{
    if (!m_model) {
        return;
    }

    connect(m_model, &Bvh3DModel::visibleChanged, this, &BvhSkeletonItem::visibleChanged);
    connect(m_model, &Bvh3DModel::jointColorChanged, this, &BvhSkeletonItem::jointColorChanged);
    connect(m_model, &Bvh3DModel::boneColorChanged, this, &BvhSkeletonItem::boneColorChanged);
    connect(m_model, &Bvh3DModel::boneColorModeChanged, this, &BvhSkeletonItem::boneColorModeChanged);
    connect(m_model, &Bvh3DModel::boneToneChanged, this, &BvhSkeletonItem::boneToneChanged);
    connect(m_model, &Bvh3DModel::customBoneColorChanged, this, &BvhSkeletonItem::customBoneColorChanged);
    connect(m_model, &Bvh3DModel::displayNameChanged, this, &BvhSkeletonItem::displayNameChanged);
    connect(m_model, &Bvh3DModel::bvhFileChanged, this, &BvhSkeletonItem::validChanged);
    connect(m_model, &Bvh3DModel::bvhFileChanged, this, &BvhSkeletonItem::frameCountChanged);
    connect(m_model, &Bvh3DModel::bvhFileChanged, this, &BvhSkeletonItem::frameTimeChanged);
    connect(m_model, &Bvh3DModel::currentFrameChanged, this, &BvhSkeletonItem::currentFrameChanged);
}
