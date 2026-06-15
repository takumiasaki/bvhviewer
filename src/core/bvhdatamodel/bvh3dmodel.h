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
    Q_PROPERTY(QColor jointColor READ jointColor WRITE setJointColor NOTIFY jointColorChanged)
    Q_PROPERTY(QColor boneColor READ boneColor NOTIFY boneColorChanged)
    Q_PROPERTY(BoneColorMode boneColorMode READ boneColorMode WRITE setBoneColorMode NOTIFY boneColorModeChanged)
    Q_PROPERTY(int boneTone READ boneTone WRITE setBoneTone NOTIFY boneToneChanged)
    Q_PROPERTY(QColor customBoneColor READ customBoneColor WRITE setCustomBoneColor NOTIFY customBoneColorChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString displayName READ displayName WRITE setDisplayName NOTIFY displayNameChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY bvhFileChanged)

public:
    enum BoneColorMode {
        SameAsJoint = 0,
        ToneOffset = 1,
        Custom = 2
    };
    Q_ENUM(BoneColorMode)

    static QColor colorFromTone(const QColor& jointColor, int tone);

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

    QColor jointColor() const { return m_jointColor; }
    void setJointColor(const QColor& color);

    QColor boneColor() const { return m_boneColor; }

    BoneColorMode boneColorMode() const { return m_boneColorMode; }
    void setBoneColorMode(BoneColorMode mode);

    int boneTone() const { return m_boneTone; }
    void setBoneTone(int tone);

    QColor customBoneColor() const { return m_customBoneColor; }
    void setCustomBoneColor(const QColor& color);

    QColor color() const { return m_jointColor; }
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
    void jointColorChanged();
    void boneColorChanged();
    void boneColorModeChanged();
    void boneToneChanged();
    void customBoneColorChanged();
    void colorChanged();
    void displayNameChanged();
    void attachFailed(const QString& reason);

private:
    QColor effectiveBoneColor() const;
    void applyEffectiveBoneColor();

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
    QColor m_jointColor = Qt::white;
    QColor m_boneColor = Qt::white;
    QColor m_customBoneColor = Qt::white;
    BoneColorMode m_boneColorMode = ToneOffset;
    int m_boneTone = -25;
    QString m_displayName;
};

#endif // BVH3DMODEL_H
