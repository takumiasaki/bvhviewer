#ifndef BVHVIEWITEM_H
#define BVHVIEWITEM_H

#include <QQuickPaintedItem>
#include "bvhscenemodel.h"
#include "bvhscenelistmodel.h"

class BvhNode;

class BvhViewItem : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(BvhSceneModel* sceneModel READ sceneModel WRITE setSceneModel NOTIFY sceneModelChanged)
    Q_PROPERTY(BvhSceneListModel* sceneManager READ sceneManager WRITE setSceneManager NOTIFY sceneManagerChanged)
    Q_PROPERTY(QColor boneColor READ boneColor WRITE setBoneColor NOTIFY boneColorChanged)

public:
    explicit BvhViewItem(QQuickItem* parent = nullptr);
    void paint(QPainter* painter) override;

    BvhSceneModel* sceneModel() const;
    void setSceneModel(BvhSceneModel* model);

    BvhSceneListModel* sceneManager() const;
    void setSceneManager(BvhSceneListModel* manager);

    QColor boneColor() const;
    void setBoneColor(const QColor& color);

signals:
    void sceneModelChanged();
    void sceneManagerChanged();
    void boneColorChanged();

private slots:
    void handleSceneModelUpdate();
    void handleSceneManagerChanged();

private:
    void drawNode(QPainter* painter, BvhNode* node, const QPointF& origin, qreal scale);
    QPointF projectPoint(const QVector3D& point, const QPointF& origin, qreal scale) const;
    void computeBounds(BvhNode* node, QVector3D& minPoint, QVector3D& maxPoint) const;
    QColor sceneColorForIndex(int index) const;

    BvhSceneModel* m_sceneModel;
    BvhSceneListModel* m_sceneManager;
    QPointer<BvhSceneModel> m_activeScene;
    QColor m_boneColor;
};

#endif // BVHVIEWITEM_H
