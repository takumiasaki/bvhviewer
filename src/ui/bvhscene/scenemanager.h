#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QAbstractListModel>
#include <QUrl>
#include <memory>
#include <vector>

#include <QtQml/qqmlregistration.h>

class Bvh3DModel;
class BvhFile;
class BvhSkeletonItem;

class SceneManager : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int activeIndex READ activeIndex WRITE setActiveIndex NOTIFY activeIndexChanged)
    Q_PROPERTY(BvhSkeletonItem* activeScene READ activeScene NOTIFY activeIndexChanged)
    Q_PROPERTY(int sceneCount READ sceneCount NOTIFY sceneCountChanged)

    Q_PROPERTY(qreal animationTime READ animationTime WRITE setAnimationTime NOTIFY animationTimeChanged)
    Q_PROPERTY(int currentFrame READ currentFrame WRITE setCurrentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(double frameTime READ frameTime NOTIFY frameTimeChanged)
    Q_PROPERTY(qreal duration READ duration NOTIFY durationChanged)

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

    Q_PROPERTY(bool floorShadowsEnabled READ floorShadowsEnabled WRITE setFloorShadowsEnabled NOTIFY floorShadowsEnabledChanged)

public:
    enum SceneRoles {
        NameRole = Qt::UserRole + 1,
        SourceRole,
        SkeletonRole,
        ValidRole,
        FrameCountRole,
        CurrentFrameRole,
        FrameTimeRole
    };
    Q_ENUM(SceneRoles)

    explicit SceneManager(QObject* parent = nullptr);
    ~SceneManager() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int activeIndex() const { return m_activeIndex; }
    void setActiveIndex(int index);

    BvhSkeletonItem* activeScene() const;
    int sceneCount() const { return static_cast<int>(m_entries.size()); }

    qreal animationTime() const { return m_animationTime; }
    void setAnimationTime(qreal time);

    int currentFrame() const { return m_currentFrame; }
    void setCurrentFrame(int frame);

    int frameCount() const;
    double frameTime() const;
    qreal duration() const;

    QString lastError() const { return m_lastError; }

    bool floorShadowsEnabled() const { return m_floorShadowsEnabled; }
    void setFloorShadowsEnabled(bool enabled);

    Q_INVOKABLE bool loadScene(const QUrl& fileUrl);
    Q_INVOKABLE bool removeScene(int index);
    Q_INVOKABLE BvhSkeletonItem* skeletonAt(int index) const;
    Q_INVOKABLE int count() const { return sceneCount(); }
    Q_INVOKABLE QColor colorForIndex(int index) const;
    Q_INVOKABLE void resetSkeletonColors(int index);

signals:
    void activeIndexChanged();
    void sceneCountChanged();
    void animationTimeChanged();
    void currentFrameChanged();
    void frameCountChanged();
    void frameTimeChanged();
    void durationChanged();
    void lastErrorChanged();
    void floorShadowsEnabledChanged();

private:
    struct SkeletonEntry {
        std::shared_ptr<BvhFile> bvhFile;
        std::unique_ptr<Bvh3DModel> model;
        BvhSkeletonItem* item = nullptr;
        QString sourcePath;
        int paletteIndex = 0;
    };

    void relayoutSceneOffsets();
    void applyAnimationTimeToVisible();
    void syncCurrentFrameFromAnimationTime();
    void syncPoseForSkeleton(BvhSkeletonItem* item);
    void emitTimelineChanged();
    void clampAnimationTimeToDuration();
    void setLastError(const QString& error);
    void connectSkeletonSignals(BvhSkeletonItem* item);
    void handleSkeletonUpdated();
    int rowForSkeleton(BvhSkeletonItem* item) const;
    static QColor defaultColorForPaletteIndex(int index);

    std::vector<SkeletonEntry> m_entries;
    int m_activeIndex = -1;
    qreal m_animationTime = 0.0;
    int m_currentFrame = -1;
    QString m_lastError;
    bool m_floorShadowsEnabled = true;
};

#endif // SCENEMANAGER_H
