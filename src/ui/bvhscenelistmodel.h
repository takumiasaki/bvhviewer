#ifndef BVHSCENELISTMODEL_H
#define BVHSCENELISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>
#include "bvhscenemodel.h"

class BvhSceneListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int activeIndex READ activeIndex WRITE setActiveIndex NOTIFY activeIndexChanged)
    Q_PROPERTY(BvhSceneModel* activeScene READ activeScene NOTIFY activeIndexChanged)
    Q_PROPERTY(qreal animationTime READ animationTime WRITE setAnimationTime NOTIFY animationTimeChanged)
    Q_PROPERTY(bool playing READ isPlaying WRITE setPlaying NOTIFY playingChanged)

public:
    enum SceneRoles {
        NameRole = Qt::UserRole + 1,
        SourceRole,
        ValidRole,
        FrameCountRole,
        CurrentFrameRole,
        FrameTimeRole
    };
    Q_ENUM(SceneRoles)

    explicit BvhSceneListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int activeIndex() const;
    void setActiveIndex(int index);
    BvhSceneModel* activeScene() const;

    Q_INVOKABLE bool loadScene(const QUrl& fileUrl);
    Q_INVOKABLE bool removeScene(int index);
    Q_INVOKABLE BvhSceneModel* sceneAt(int index) const;
    Q_INVOKABLE int count() const;
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void toggle();

    qreal animationTime() const { return m_animationTime; }
    void setAnimationTime(qreal t);

    bool isPlaying() const { return m_playing; }
    void setPlaying(bool p);

signals:
    void activeIndexChanged();
    void animationTimeChanged();
    void playingChanged();

private slots:
    void handleSceneUpdated();

private:
    void connectSceneSignals(BvhSceneModel* scene);
    int rowForScene(BvhSceneModel* scene) const;

    QVector<BvhSceneModel*> m_scenes;
    int m_activeIndex;
    qreal m_animationTime;
    bool m_playing;
};

#endif // BVHSCENELISTMODEL_H
