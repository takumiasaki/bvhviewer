#ifndef BVHSCENEMODEL_H
#define BVHSCENEMODEL_H

#include <QObject>
#include <QUrl>
#include <QPointF>
#include "../core/bvhfile.h"

class BvhSceneModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(int currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(double frameTime READ frameTime NOTIFY frameTimeChanged)
    Q_PROPERTY(QUrl source READ source NOTIFY sourceChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(QPointF offset READ offset WRITE setOffset NOTIFY offsetChanged)

public:
    explicit BvhSceneModel(QObject* parent = nullptr);

    bool isValid() const;
    int currentFrame() const;
    int frameCount() const;
    double frameTime() const;
    QUrl source() const;
    QString errorString() const;

    Q_INVOKABLE bool load(const QUrl& fileUrl);
    BvhNode* getRootNode() const;

    QPointF offset() const;
    void setOffset(const QPointF& ofs);

public slots:
    void setCurrentFrame(int frameIndex);

signals:
    void validChanged();
    void currentFrameChanged();
    void frameCountChanged();
    void frameTimeChanged();
    void sourceChanged();
    void errorStringChanged();
    void offsetChanged();

private:
    BvhFile bvh;
    int currentFrameIndex;
    QUrl sourceUrl;
    QString errorStringValue;
    QPointF m_offset;
};

#endif // BVHSCENEMODEL_H
