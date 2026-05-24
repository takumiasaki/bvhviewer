#include "bvhscenemodel.h"
#include <QFileInfo>
#include <QDebug>

BvhSceneModel::BvhSceneModel(QObject* parent)
    : QObject(parent), currentFrameIndex(-1), m_offset(0.0, 0.0)
{
}

bool BvhSceneModel::isValid() const
{
    return bvh.isValid();
}

int BvhSceneModel::currentFrame() const
{
    return currentFrameIndex;
}

int BvhSceneModel::frameCount() const
{
    return bvh.getFrameCount();
}

double BvhSceneModel::frameTime() const
{
    return bvh.getFrameTime();
}

QUrl BvhSceneModel::source() const
{
    return sourceUrl;
}

QString BvhSceneModel::errorString() const
{
    return errorStringValue;
}

QPointF BvhSceneModel::offset() const
{
    return m_offset;
}

void BvhSceneModel::setOffset(const QPointF& ofs)
{
    if (m_offset == ofs)
        return;
    m_offset = ofs;
    emit offsetChanged();
}

bool BvhSceneModel::load(const QUrl& fileUrl)
{
    qDebug() << "Loading BVH file from URL:" << fileUrl;
    errorStringValue.clear();
    emit errorStringChanged();

    if (!fileUrl.isLocalFile()) {
        errorStringValue = QStringLiteral("Selected file is not local: %1").arg(fileUrl.toString());
        emit errorStringChanged();
        qWarning() << errorStringValue;
        return false;
    }

    QString filePath = fileUrl.toLocalFile();
    QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        errorStringValue = QStringLiteral("BVH file does not exist: %1").arg(filePath);
        emit errorStringChanged();
        qWarning() << errorStringValue;
        return false;
    }

    if (!bvh.load(filePath)) {
        errorStringValue = QStringLiteral("Failed to parse BVH file: %1").arg(filePath);
        emit errorStringChanged();
        return false;
    }

    sourceUrl = fileUrl;
    emit sourceChanged();

    currentFrameIndex = -1;
    if (bvh.getFrameCount() > 0) {
        setCurrentFrame(0);
    }

    emit validChanged();
    emit frameCountChanged();
    emit frameTimeChanged();
    errorStringValue.clear();
    emit errorStringChanged();
    return true;
}

BvhNode* BvhSceneModel::getRootNode() const
{
    return bvh.getRootNode();
}

void BvhSceneModel::setCurrentFrame(int frameIndex)
{
    if (frameIndex == currentFrameIndex) {
        return;
    }

    if (!bvh.isValid()) {
        return;
    }

    if (frameIndex < 0 || frameIndex >= bvh.getFrameCount()) {
        qWarning() << "Invalid frame index:" << frameIndex;
        return;
    }

    currentFrameIndex = frameIndex;
    bvh.setCurrentFrame(frameIndex);
    emit currentFrameChanged();
}
