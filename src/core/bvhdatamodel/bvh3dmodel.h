#ifndef BVH3DMODEL_H
#define BVH3DMODEL_H

#include <QColor>
#include <QObject>
#include <memory>

#include "bvh3dbone.h"
#include "bvh3djoint.h"
#include "bvhbonelistmodel.h"
#include "bvhjointlistmodel.h"

class BvhFile;

class Bvh3DModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(BvhJointListModel* jointModel READ jointModel CONSTANT)
    Q_PROPERTY(BvhBoneListModel* boneModel READ boneModel CONSTANT)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY bvhFileChanged)
    Q_PROPERTY(double frameTime READ frameTime NOTIFY bvhFileChanged)
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY currentFrameChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY bvhFileChanged)

public:
    explicit Bvh3DModel(QObject* parent = nullptr);
    explicit Bvh3DModel(std::shared_ptr<BvhFile> bvhFile, QObject* parent = nullptr);
    ~Bvh3DModel() override;

    bool attach(std::shared_ptr<BvhFile> bvhFile);
    std::shared_ptr<BvhFile> bvhFile() const { return m_bvhFile; }
    bool isValid() const;

    Q_INVOKABLE void setFrame(int frameIndex);
    Q_INVOKABLE void setPoseAtTime(double seconds);

    BvhJointListModel* jointModel() const { return m_jointModel; }
    BvhBoneListModel* boneModel() const { return m_boneModel; }

    int frameCount() const;
    double frameTime() const;
    int currentFrame() const { return m_currentFrame; }

    bool visible() const { return m_visible; }
    void setVisible(bool visible);

    QColor color() const { return m_color; }
    void setColor(const QColor& color);

    QString displayName() const { return m_displayName; }
    void setDisplayName(const QString& name);

    int jointCount() const { return m_joints.size(); }
    int boneCount() const { return m_bones.size(); }

    const QVector<Bvh3DJoint>& joints() const { return m_joints; }
    const QVector<Bvh3DBone>& bones() const { return m_bones; }

signals:
    void bvhFileChanged();
    void currentFrameChanged();
    void poseUpdated();
    void visibleChanged();
    void colorChanged();
    void displayNameChanged();
    void attachFailed(const QString& reason);

private:
    void buildSkeletonMetadata();
    void updatePoseFromBvhFile(bool immediateNotify = false);
    void flushPoseRefresh();
    void notifyPoseChanged();
    void applyInitialPose();
    void reset();

    std::shared_ptr<BvhFile> m_bvhFile;
    QVector<Bvh3DJoint> m_joints;
    QVector<Bvh3DBone> m_bones;
    BvhJointListModel* m_jointModel = nullptr;
    BvhBoneListModel* m_boneModel = nullptr;

    int m_currentFrame = -1;
    bool m_poseRefreshScheduled = false;
    bool m_visible = true;
    QColor m_color = Qt::white;
    QString m_displayName;
};

#endif // BVH3DMODEL_H
