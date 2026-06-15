#ifndef BVHSKELETONITEM_H
#define BVHSKELETONITEM_H

#include <QColor>
#include <QObject>
#include <QUrl>
#include <QVector3D>

#include <QtQml/qqmlregistration.h>

#include "bvh3dmodel.h"
#include "bvhbonelistmodel.h"
#include "bvhjointlistmodel.h"

class BvhSkeletonItem : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("BvhSkeletonItem instances are created by SceneManager")
    Q_PROPERTY(Bvh3DModel* model READ model CONSTANT)
    Q_PROPERTY(BvhJointListModel* jointModel READ jointModel CONSTANT)
    Q_PROPERTY(BvhBoneListModel* boneModel READ boneModel CONSTANT)
    Q_PROPERTY(QVector3D sceneOffset READ sceneOffset WRITE setSceneOffset NOTIFY sceneOffsetChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(QColor jointColor READ jointColor WRITE setJointColor NOTIFY jointColorChanged)
    Q_PROPERTY(QColor boneColor READ boneColor NOTIFY boneColorChanged)
    Q_PROPERTY(Bvh3DModel::BoneColorMode boneColorMode READ boneColorMode WRITE setBoneColorMode NOTIFY boneColorModeChanged)
    Q_PROPERTY(int boneTone READ boneTone WRITE setBoneTone NOTIFY boneToneChanged)
    Q_PROPERTY(QColor customBoneColor READ customBoneColor WRITE setCustomBoneColor NOTIFY customBoneColorChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int paletteIndex READ paletteIndex CONSTANT)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(QUrl source READ source CONSTANT)
    Q_PROPERTY(QString sourcePath READ sourcePath CONSTANT)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(double frameTime READ frameTime NOTIFY frameTimeChanged)
    Q_PROPERTY(int currentFrame READ currentFrame NOTIFY currentFrameChanged)

public:
    explicit BvhSkeletonItem(Bvh3DModel* model,
                               const QString& sourcePath,
                               int paletteIndex,
                               QObject* parent = nullptr);

    Bvh3DModel* model() const { return m_model; }
    BvhJointListModel* jointModel() const;
    BvhBoneListModel* boneModel() const;

    QVector3D sceneOffset() const { return m_sceneOffset; }
    void setSceneOffset(const QVector3D& offset);

    bool visible() const;
    void setVisible(bool visible);

    QColor jointColor() const;
    void setJointColor(const QColor& color);

    QColor boneColor() const;

    Bvh3DModel::BoneColorMode boneColorMode() const;
    void setBoneColorMode(Bvh3DModel::BoneColorMode mode);

    int boneTone() const;
    void setBoneTone(int tone);

    QColor customBoneColor() const;
    void setCustomBoneColor(const QColor& color);

    QColor color() const;
    void setColor(const QColor& color);

    int paletteIndex() const { return m_paletteIndex; }

    QString displayName() const;
    QUrl source() const;
    QString sourcePath() const { return m_sourcePath; }

    bool isValid() const;
    int frameCount() const;
    double frameTime() const;
    int currentFrame() const;

signals:
    void sceneOffsetChanged();
    void visibleChanged();
    void jointColorChanged();
    void boneColorChanged();
    void boneColorModeChanged();
    void boneToneChanged();
    void customBoneColorChanged();
    void colorChanged();
    void displayNameChanged();
    void validChanged();
    void frameCountChanged();
    void frameTimeChanged();
    void currentFrameChanged();

private:
    void connectModelSignals();

    Bvh3DModel* m_model = nullptr;
    QString m_sourcePath;
    QVector3D m_sceneOffset;
    int m_paletteIndex = 0;
};

#endif // BVHSKELETONITEM_H
