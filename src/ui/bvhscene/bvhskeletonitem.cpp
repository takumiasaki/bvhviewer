#include "bvhskeletonitem.h"

#include <QFileInfo>

BvhSkeletonItem::BvhSkeletonItem(Bvh3DModel* model, const QString& sourcePath, QObject* parent)
    : QObject(parent)
    , m_model(model)
    , m_sourcePath(sourcePath)
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

QColor BvhSkeletonItem::color() const
{
    return m_model ? m_model->color() : QColor(Qt::white);
}

void BvhSkeletonItem::setColor(const QColor& color)
{
    if (m_model) {
        m_model->setColor(color);
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
    connect(m_model, &Bvh3DModel::colorChanged, this, &BvhSkeletonItem::colorChanged);
    connect(m_model, &Bvh3DModel::displayNameChanged, this, &BvhSkeletonItem::displayNameChanged);
    connect(m_model, &Bvh3DModel::bvhFileChanged, this, &BvhSkeletonItem::validChanged);
    connect(m_model, &Bvh3DModel::bvhFileChanged, this, &BvhSkeletonItem::frameCountChanged);
    connect(m_model, &Bvh3DModel::bvhFileChanged, this, &BvhSkeletonItem::frameTimeChanged);
    connect(m_model, &Bvh3DModel::currentFrameChanged, this, &BvhSkeletonItem::currentFrameChanged);
}
