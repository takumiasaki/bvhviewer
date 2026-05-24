#include "bvhviewitem.h"
#include "bvhscenemodel.h"
#include "bvhscenelistmodel.h"
#include "../core/bvhnode.h"
#include <QPainter>
#include <QDebug>
#include <QPointer>

BvhViewItem::BvhViewItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , m_sceneModel(nullptr)
    , m_sceneManager(nullptr)
    , m_boneColor(Qt::white)
{
    setAntialiasing(true);
}

BvhSceneModel* BvhViewItem::sceneModel() const
{
    return m_sceneModel;
}

void BvhViewItem::setSceneModel(BvhSceneModel* model)
{
    if (m_sceneModel == model) {
        return;
    }

    if (m_sceneModel) {
        disconnect(m_sceneModel, nullptr, this, nullptr);
    }

    m_sceneModel = model;
    if (m_sceneModel) {
        connect(m_sceneModel, &BvhSceneModel::currentFrameChanged, this, &BvhViewItem::handleSceneModelUpdate);
        connect(m_sceneModel, &BvhSceneModel::validChanged, this, &BvhViewItem::handleSceneModelUpdate);
        connect(m_sceneModel, &BvhSceneModel::frameCountChanged, this, &BvhViewItem::handleSceneModelUpdate);
    }

    emit sceneModelChanged();
    update();
}

BvhSceneListModel* BvhViewItem::sceneManager() const
{
    return m_sceneManager;
}

void BvhViewItem::setSceneManager(BvhSceneListModel* manager)
{
    if (m_sceneManager == manager) {
        return;
    }

    if (m_sceneManager) {
        disconnect(m_sceneManager, nullptr, this, nullptr);
    }

    m_sceneManager = manager;
    if (m_sceneManager) {
        connect(m_sceneManager, &BvhSceneListModel::rowsInserted, this, &BvhViewItem::handleSceneManagerChanged);
        connect(m_sceneManager, &BvhSceneListModel::rowsRemoved, this, &BvhViewItem::handleSceneManagerChanged);
        connect(m_sceneManager, &BvhSceneListModel::modelReset, this, &BvhViewItem::handleSceneManagerChanged);
        connect(m_sceneManager, &BvhSceneListModel::activeIndexChanged, this, &BvhViewItem::handleSceneManagerChanged);
    }

    emit sceneManagerChanged();
    update();
    qDebug() << "BvhViewItem::setSceneManager - manager set:" << manager;
    // Initialize active scene connections
    handleSceneManagerChanged();
}

QColor BvhViewItem::boneColor() const
{
    return m_boneColor;
}

void BvhViewItem::setBoneColor(const QColor& color)
{
    if (m_boneColor == color) {
        return;
    }
    m_boneColor = color;
    emit boneColorChanged();
    update();
}

void BvhViewItem::handleSceneModelUpdate()
{
    qDebug() << "BvhViewItem::handleSceneModelUpdate - updating view";
    update();
}

void BvhViewItem::handleSceneManagerChanged()
{
    qDebug() << "BvhViewItem::handleSceneManagerChanged - reconnecting active scene";

    // disconnect previous active scene safely
    if (!m_activeScene.isNull()) {
        qDebug() << "disconnecting previous active scene:" << m_activeScene.data();
        disconnect(m_activeScene.data(), nullptr, this, nullptr);
        m_activeScene.clear();
    }

    if (m_sceneManager) {
        BvhSceneModel* active = m_sceneManager->activeScene();
        if (active) {
            qDebug() << "connecting to new active scene:" << active;
            m_activeScene = active;
            connect(m_activeScene.data(), &BvhSceneModel::currentFrameChanged, this, &BvhViewItem::handleSceneModelUpdate);
            connect(m_activeScene.data(), &BvhSceneModel::validChanged, this, &BvhViewItem::handleSceneModelUpdate);
            connect(m_activeScene.data(), &BvhSceneModel::frameCountChanged, this, &BvhViewItem::handleSceneModelUpdate);
        } else {
            qDebug() << "no active scene in manager";
        }
    }

    update();
}

void BvhViewItem::paint(QPainter* painter)
{
    painter->fillRect(boundingRect(), Qt::black);

    if (m_sceneManager && m_sceneManager->rowCount() > 0) {
        QVector3D minPoint(1e9, 1e9, 1e9);
        QVector3D maxPoint(-1e9, -1e9, -1e9);
        QVector<BvhSceneModel*> scenes;

        for (int i = 0; i < m_sceneManager->rowCount(); ++i) {
            BvhSceneModel* scene = m_sceneManager->sceneAt(i);
            if (!scene || !scene->isValid()) {
                continue;
            }
            BvhNode* root = scene->getRootNode();
            if (!root) {
                continue;
            }
            scenes.append(scene);
            computeBounds(root, minPoint, maxPoint);
        }

        if (scenes.isEmpty()) {
            painter->setPen(Qt::white);
            painter->drawText(boundingRect(), Qt::AlignCenter, QLatin1String("No BVH loaded"));
            return;
        }

        qreal frameWidth = qMax(1.0, maxPoint.x() - minPoint.x());
        qreal frameHeight = qMax(1.0, maxPoint.y() - minPoint.y());
        qreal margin = 0.9;
        qreal scaleX = (width() * margin) / frameWidth;
        qreal scaleY = (height() * margin) / frameHeight;
        qreal scale = qMin(scaleX, scaleY);
        if (scale <= 0.0) {
            scale = qMin(width(), height()) / 200.0;
        }

        qreal centerX = (minPoint.x() + maxPoint.x()) * 0.5;
        qreal centerY = (minPoint.y() + maxPoint.y()) * 0.5;
        QPointF origin(width() * 0.5 - centerX * scale,
                       height() * 0.5 + centerY * scale);

        painter->setRenderHint(QPainter::Antialiasing);
        for (int i = 0; i < scenes.size(); ++i) {
            BvhSceneModel* scene = scenes.at(i);
            BvhNode* root = scene->getRootNode();
            painter->setPen(QPen(sceneColorForIndex(i), 2.0));
            QPointF sceneOff(0,0);
            // apply per-scene offset (in world units) scaled to view
            sceneOff = scene->offset();
            QPointF shiftedOrigin = origin + QPointF(sceneOff.x() * scale, -sceneOff.y() * scale);
            drawNode(painter, root, shiftedOrigin, scale);
        }
        return;
    }

    if (!m_sceneModel || !m_sceneModel->isValid()) {
        painter->setPen(Qt::white);
        painter->drawText(boundingRect(), Qt::AlignCenter, QLatin1String("No BVH loaded"));
        return;
    }

    BvhNode* root = m_sceneModel->getRootNode();
    if (!root) {
        return;
    }

    QVector3D minPoint(1e9, 1e9, 1e9);
    QVector3D maxPoint(-1e9, -1e9, -1e9);
    computeBounds(root, minPoint, maxPoint);

    qreal frameWidth = qMax(1.0, maxPoint.x() - minPoint.x());
    qreal frameHeight = qMax(1.0, maxPoint.y() - minPoint.y());
    qreal margin = 0.9;
    qreal scaleX = (width() * margin) / frameWidth;
    qreal scaleY = (height() * margin) / frameHeight;
    qreal scale = qMin(scaleX, scaleY);
    if (scale <= 0.0) {
        scale = qMin(width(), height()) / 200.0;
    }

    qreal centerX = (minPoint.x() + maxPoint.x()) * 0.5;
    qreal centerY = (minPoint.y() + maxPoint.y()) * 0.5;
    QPointF origin(width() * 0.5 - centerX * scale,
                   height() * 0.5 + centerY * scale);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(m_boneColor, 2.0));
    drawNode(painter, root, origin, scale);
}

QPointF BvhViewItem::projectPoint(const QVector3D& point, const QPointF& origin, qreal scale) const
{
    return QPointF(origin.x() + point.x() * scale,
                   origin.y() - point.y() * scale);
}

void BvhViewItem::computeBounds(BvhNode* node, QVector3D& minPoint, QVector3D& maxPoint) const
{
    if (!node) {
        return;
    }

    QVector3D position = node->getWorldPosition();
    minPoint.setX(qMin(minPoint.x(), position.x()));
    minPoint.setY(qMin(minPoint.y(), position.y()));
    minPoint.setZ(qMin(minPoint.z(), position.z()));
    maxPoint.setX(qMax(maxPoint.x(), position.x()));
    maxPoint.setY(qMax(maxPoint.y(), position.y()));
    maxPoint.setZ(qMax(maxPoint.z(), position.z()));

    if (node->hasEndSiteInfo()) {
        QVector3D endPosition = node->getWorldTransform().map(node->getEndSiteOffset());
        minPoint.setX(qMin(minPoint.x(), endPosition.x()));
        minPoint.setY(qMin(minPoint.y(), endPosition.y()));
        minPoint.setZ(qMin(minPoint.z(), endPosition.z()));
        maxPoint.setX(qMax(maxPoint.x(), endPosition.x()));
        maxPoint.setY(qMax(maxPoint.y(), endPosition.y()));
        maxPoint.setZ(qMax(maxPoint.z(), endPosition.z()));
    }

    for (BvhNode* child : node->getChildren()) {
        computeBounds(child, minPoint, maxPoint);
    }
}

void BvhViewItem::drawNode(QPainter* painter, BvhNode* node, const QPointF& origin, qreal scale)
{
    if (!node) {
        return;
    }

    QVector3D startPosition = node->getWorldPosition();
    QPointF startPoint = projectPoint(startPosition, origin, scale);

    for (BvhNode* child : node->getChildren()) {
        if (!child) {
            continue;
        }
        QVector3D childPosition = child->getWorldPosition();
        QPointF childPoint = projectPoint(childPosition, origin, scale);
        painter->drawLine(startPoint, childPoint);
        drawNode(painter, child, origin, scale);
    }

    if (node->hasEndSiteInfo()) {
        QVector3D endPosition = node->getWorldTransform().map(node->getEndSiteOffset());
        QPointF endPoint = projectPoint(endPosition, origin, scale);
        painter->drawLine(startPoint, endPoint);
    }
}

QColor BvhViewItem::sceneColorForIndex(int index) const
{
    static const QVector<QColor> palette = {
        QColor("#F4F1BB"),
        QColor("#9BC1BC"),
        QColor("#5A8F7B"),
        QColor("#EE6C4D"),
        QColor("#3D5A80"),
        QColor("#98C1D9")
    };
    return palette.value(index % palette.size(), Qt::white);
}
