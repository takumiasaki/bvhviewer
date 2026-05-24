#include "bvhscenelistmodel.h"
#include "bvhscenemodel.h"
#include <QFileInfo>
#include <QDebug>
#include <cmath>

BvhSceneListModel::BvhSceneListModel(QObject* parent)
    : QAbstractListModel(parent), m_activeIndex(-1)
{
    m_animationTime = 0.0;
    m_playing = false;
}

int BvhSceneListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_scenes.size();
}

int BvhSceneListModel::count() const
{
    return m_scenes.size();
}

QVariant BvhSceneListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_scenes.size()) {
        return QVariant();
    }

    BvhSceneModel* scene = m_scenes.at(index.row());
    if (!scene) {
        return QVariant();
    }

    switch (role) {
    case NameRole: {
        QFileInfo info(scene->source().toLocalFile());
        return info.fileName();
    }
    case SourceRole:
        return scene->source();
    case ValidRole:
        return scene->isValid();
    case FrameCountRole:
        return scene->frameCount();
    case CurrentFrameRole:
        return scene->currentFrame();
    case FrameTimeRole:
        return scene->frameTime();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> BvhSceneListModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[SourceRole] = "source";
    roles[ValidRole] = "valid";
    roles[FrameCountRole] = "frameCount";
    roles[CurrentFrameRole] = "currentFrame";
    roles[FrameTimeRole] = "frameTime";
    return roles;
}

int BvhSceneListModel::activeIndex() const
{
    return m_activeIndex;
}

void BvhSceneListModel::setActiveIndex(int index)
{
    if (index == m_activeIndex) {
        return;
    }

    if (index < -1 || index >= m_scenes.size()) {
        qWarning() << "Invalid active scene index:" << index;
        return;
    }

    m_activeIndex = index;
    emit activeIndexChanged();
}

BvhSceneModel* BvhSceneListModel::activeScene() const
{
    if (m_activeIndex >= 0 && m_activeIndex < m_scenes.size()) {
        return m_scenes.at(m_activeIndex);
    }
    return nullptr;
}

bool BvhSceneListModel::loadScene(const QUrl& fileUrl)
{
    if (!fileUrl.isLocalFile()) {
        qWarning() << "Scene file must be local:" << fileUrl;
        return false;
    }

    BvhSceneModel* scene = new BvhSceneModel(this);
    if (!scene->load(fileUrl)) {
        delete scene;
        return false;
    }

    const int insertIndex = m_scenes.size();
    beginInsertRows(QModelIndex(), insertIndex, insertIndex);
    m_scenes.append(scene);
    endInsertRows();
    connectSceneSignals(scene);

    // Assign a spaced offset so scenes don't overlap visually
    int size = m_scenes.size();
    double center = (size - 1) / 2.0;
    for (int i = 0; i < size; ++i) {
        BvhSceneModel* s = m_scenes.at(i);
        if (s) {
            double x = (i - center) * 200.0; // 200 units spacing
            s->setOffset(QPointF(x, 0.0));
        }
    }

    if (m_activeIndex == -1) {
        setActiveIndex(insertIndex);
    }

    return true;
}

bool BvhSceneListModel::removeScene(int index)
{
    if (index < 0 || index >= m_scenes.size()) {
        return false;
    }

    beginRemoveRows(QModelIndex(), index, index);
    BvhSceneModel* scene = m_scenes.takeAt(index);
    endRemoveRows();

    if (scene) {
        scene->deleteLater();
    }

    if (m_scenes.isEmpty()) {
        setActiveIndex(-1);
    } else if (m_activeIndex == index) {
        setActiveIndex(qMin(index, m_scenes.size() - 1));
    } else if (m_activeIndex > index) {
        setActiveIndex(m_activeIndex - 1);
    }

    // Recompute offsets after removal
    int size = m_scenes.size();
    double center = size > 0 ? (size - 1) / 2.0 : 0.0;
    for (int i = 0; i < size; ++i) {
        BvhSceneModel* s = m_scenes.at(i);
        if (s) {
            double x = (i - center) * 200.0;
            s->setOffset(QPointF(x, 0.0));
        }
    }

    return true;
}

BvhSceneModel* BvhSceneListModel::sceneAt(int index) const
{
    if (index < 0 || index >= m_scenes.size()) {
        return nullptr;
    }
    return m_scenes.at(index);
}

void BvhSceneListModel::setAnimationTime(qreal t)
{
    if (qFuzzyCompare(t + 1.0, m_animationTime + 1.0)) {
        return;
    }
    m_animationTime = t;
    // Map global time (seconds) to frames for each scene
    for (BvhSceneModel* s : qAsConst(m_scenes)) {
        if (!s || !s->isValid() || s->frameCount() <= 0) continue;
        qreal ft = s->frameTime();
        if (ft <= 0.0) continue;
        int frame = static_cast<int>(std::floor(m_animationTime / ft));
        if (s->frameCount() > 0) {
            frame = frame % s->frameCount();
            if (frame < 0) frame += s->frameCount();
        }
        if (s->currentFrame() != frame) {
            s->setCurrentFrame(frame);
        }
    }
    emit animationTimeChanged();
}

void BvhSceneListModel::setPlaying(bool p)
{
    if (m_playing == p) return;
    m_playing = p;
    emit playingChanged();
}

void BvhSceneListModel::play()
{
    setPlaying(true);
}

void BvhSceneListModel::pause()
{
    setPlaying(false);
}

void BvhSceneListModel::toggle()
{
    setPlaying(!m_playing);
}

void BvhSceneListModel::handleSceneUpdated()
{
    BvhSceneModel* scene = qobject_cast<BvhSceneModel*>(sender());
    int row = rowForScene(scene);
    if (row < 0) {
        return;
    }

    QModelIndex modelIndex = index(row);
    emit dataChanged(modelIndex, modelIndex);
}

void BvhSceneListModel::connectSceneSignals(BvhSceneModel* scene)
{
    if (!scene) {
        return;
    }

    connect(scene, &BvhSceneModel::currentFrameChanged, this, &BvhSceneListModel::handleSceneUpdated);
    connect(scene, &BvhSceneModel::frameCountChanged, this, &BvhSceneListModel::handleSceneUpdated);
    connect(scene, &BvhSceneModel::frameTimeChanged, this, &BvhSceneListModel::handleSceneUpdated);
    connect(scene, &BvhSceneModel::sourceChanged, this, &BvhSceneListModel::handleSceneUpdated);
    connect(scene, &BvhSceneModel::validChanged, this, &BvhSceneListModel::handleSceneUpdated);
}

int BvhSceneListModel::rowForScene(BvhSceneModel* scene) const
{
    return m_scenes.indexOf(scene);
}
