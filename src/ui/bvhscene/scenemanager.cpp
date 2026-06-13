#include "scenemanager.h"

#include "bvhskeletonitem.h"
#include "bvh3dmodel.h"
#include "bvhfile.h"

#include <QFileInfo>
#include <QtMath>

namespace {

constexpr qreal kSceneSpacing = 200.0;

const QColor kPalette[] = {
    QColor(QStringLiteral("#67C1FF")),
    QColor(QStringLiteral("#FF9F5E")),
    QColor(QStringLiteral("#7BF59F")),
    QColor(QStringLiteral("#D17BFF")),
    QColor(QStringLiteral("#FFD464")),
};
constexpr int kPaletteSize = sizeof(kPalette) / sizeof(kPalette[0]);

} // namespace

SceneManager::SceneManager(QObject* parent)
    : QAbstractListModel(parent)
{
}

SceneManager::~SceneManager() = default;

int SceneManager::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_entries.size());
}

QVariant SceneManager::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(m_entries.size())) {
        return QVariant();
    }

    const SkeletonEntry& entry = m_entries.at(static_cast<size_t>(index.row()));
    const BvhSkeletonItem* item = entry.item;
    if (!item) {
        return QVariant();
    }

    switch (role) {
    case NameRole:
        return item->displayName();
    case SourceRole:
        return item->source();
    case SkeletonRole:
        return QVariant::fromValue(entry.item);
    case ValidRole:
        return item->isValid();
    case FrameCountRole:
        return item->frameCount();
    case CurrentFrameRole:
        return item->currentFrame();
    case FrameTimeRole:
        return item->frameTime();
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> SceneManager::roleNames() const
{
    return {
        {NameRole, "name"},
        {SourceRole, "source"},
        {SkeletonRole, "skeleton"},
        {ValidRole, "valid"},
        {FrameCountRole, "frameCount"},
        {CurrentFrameRole, "currentFrame"},
        {FrameTimeRole, "frameTime"},
    };
}

void SceneManager::setActiveIndex(int index)
{
    if (index == m_activeIndex) {
        return;
    }

    if (index < -1 || index >= static_cast<int>(m_entries.size())) {
        qWarning() << "Invalid active scene index:" << index;
        return;
    }

    m_activeIndex = index;
    syncCurrentFrameFromModels();
    emit activeIndexChanged();
    emit frameCountChanged();
    emit frameTimeChanged();
}

BvhSkeletonItem* SceneManager::activeScene() const
{
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_entries.size())) {
        return m_entries.at(static_cast<size_t>(m_activeIndex)).item;
    }
    return nullptr;
}

int SceneManager::frameCount() const
{
    const BvhSkeletonItem* active = activeScene();
    return active ? active->frameCount() : 0;
}

double SceneManager::frameTime() const
{
    const BvhSkeletonItem* active = activeScene();
    return active ? active->frameTime() : 0.0;
}

bool SceneManager::loadScene(const QUrl& fileUrl)
{
    setLastError({});

    if (!fileUrl.isLocalFile()) {
        setLastError(QStringLiteral("Selected file is not local: %1").arg(fileUrl.toString()));
        return false;
    }

    const QString filePath = fileUrl.toLocalFile();
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile()) {
        setLastError(QStringLiteral("BVH file does not exist: %1").arg(filePath));
        return false;
    }

    auto bvhFile = std::make_shared<BvhFile>();
    if (!bvhFile->load(filePath)) {
        setLastError(QStringLiteral("Failed to parse BVH file: %1").arg(filePath));
        return false;
    }

    auto model = std::make_unique<Bvh3DModel>(bvhFile);
    model->setDisplayName(info.completeBaseName());
    model->setColor(colorForIndex(static_cast<int>(m_entries.size())));

    const int insertIndex = static_cast<int>(m_entries.size());
    beginInsertRows(QModelIndex(), insertIndex, insertIndex);

    SkeletonEntry entry;
    entry.bvhFile = std::move(bvhFile);
    entry.model = std::move(model);
    entry.sourcePath = filePath;
    entry.item = new BvhSkeletonItem(entry.model.get(), filePath, this);
    connectSkeletonSignals(entry.item);

    m_entries.push_back(std::move(entry));
    endInsertRows();

    relayoutSceneOffsets();

    if (m_activeIndex == -1) {
        setActiveIndex(insertIndex);
    }

    if (m_currentFrame < 0 && m_entries.back().model->frameCount() > 0) {
        setCurrentFrame(0);
    } else if (m_currentFrame >= 0) {
        m_entries.back().model->setFrame(m_currentFrame);
    }

    emit sceneCountChanged();
    emit frameCountChanged();
    emit frameTimeChanged();
    return true;
}

bool SceneManager::removeScene(int index)
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) {
        return false;
    }

    beginRemoveRows(QModelIndex(), index, index);

    BvhSkeletonItem* item = m_entries.at(static_cast<size_t>(index)).item;
    if (item) {
        item->deleteLater();
    }
    m_entries.erase(m_entries.begin() + index);
    endRemoveRows();

    if (m_entries.empty()) {
        setActiveIndex(-1);
        m_currentFrame = -1;
        emit currentFrameChanged();
    } else if (m_activeIndex == index) {
        setActiveIndex(qMin(index, static_cast<int>(m_entries.size()) - 1));
    } else if (m_activeIndex > index) {
        m_activeIndex -= 1;
        emit activeIndexChanged();
    }

    relayoutSceneOffsets();
    emit sceneCountChanged();
    emit frameCountChanged();
    emit frameTimeChanged();
    return true;
}

BvhSkeletonItem* SceneManager::skeletonAt(int index) const
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) {
        return nullptr;
    }
    return m_entries.at(static_cast<size_t>(index)).item;
}

void SceneManager::setPlaying(bool playing)
{
    if (m_playing == playing) {
        return;
    }
    m_playing = playing;
    emit playingChanged();
}

void SceneManager::setAnimationTime(qreal time)
{
    if (qFuzzyCompare(time + 1.0, m_animationTime + 1.0)) {
        return;
    }

    m_animationTime = time;
    applyAnimationTimeToAll();
    syncCurrentFrameFromModels();
    emit animationTimeChanged();
}

void SceneManager::setCurrentFrame(int frame)
{
    if (frame == m_currentFrame) {
        return;
    }

    m_currentFrame = frame;

    for (const SkeletonEntry& entry : m_entries) {
        if (entry.model && entry.model->isValid() && entry.model->frameCount() > 0) {
            entry.model->setFrame(frame);
        }
    }

    const double ft = frameTime();
    if (ft > 0.0) {
        m_animationTime = frame * ft;
        emit animationTimeChanged();
    }

    emit currentFrameChanged();
}

void SceneManager::play()
{
    setPlaying(true);
}

void SceneManager::pause()
{
    setPlaying(false);
}

void SceneManager::toggle()
{
    setPlaying(!m_playing);
}

void SceneManager::relayoutSceneOffsets()
{
    const int size = static_cast<int>(m_entries.size());
    const double center = size > 0 ? (size - 1) / 2.0 : 0.0;

    for (int i = 0; i < size; ++i) {
        BvhSkeletonItem* item = m_entries.at(static_cast<size_t>(i)).item;
        if (!item) {
            continue;
        }
        const double x = (i - center) * kSceneSpacing;
        item->setSceneOffset(QVector3D(static_cast<float>(x), 0.0f, 0.0f));
    }
}

void SceneManager::applyAnimationTimeToAll()
{
    for (const SkeletonEntry& entry : m_entries) {
        if (entry.model && entry.model->isValid()) {
            entry.model->setPoseAtTime(m_animationTime);
        }
    }
}

void SceneManager::syncCurrentFrameFromModels()
{
    const BvhSkeletonItem* active = activeScene();
    const int frame = active ? active->currentFrame() : -1;
    if (frame == m_currentFrame) {
        return;
    }
    m_currentFrame = frame;
    emit currentFrameChanged();
}

void SceneManager::setLastError(const QString& error)
{
    if (m_lastError == error) {
        return;
    }
    m_lastError = error;
    emit lastErrorChanged();
}

void SceneManager::connectSkeletonSignals(BvhSkeletonItem* item)
{
    if (!item) {
        return;
    }

    connect(item, &BvhSkeletonItem::displayNameChanged, this, &SceneManager::handleSkeletonUpdated);
    connect(item, &BvhSkeletonItem::validChanged, this, &SceneManager::handleSkeletonUpdated);
    connect(item, &BvhSkeletonItem::frameCountChanged, this, [this]() {
        emit frameCountChanged();
        handleSkeletonUpdated();
    });
    connect(item, &BvhSkeletonItem::frameTimeChanged, this, [this]() {
        emit frameTimeChanged();
        handleSkeletonUpdated();
    });
    connect(item, &BvhSkeletonItem::currentFrameChanged, this, [this, item]() {
        if (item == activeScene()) {
            syncCurrentFrameFromModels();
        }
    });
}

void SceneManager::handleSkeletonUpdated()
{
    auto* item = qobject_cast<BvhSkeletonItem*>(sender());
    const int row = rowForSkeleton(item);
    if (row < 0) {
        return;
    }

    const QModelIndex modelIndex = index(row);
    emit dataChanged(modelIndex, modelIndex);
}

int SceneManager::rowForSkeleton(BvhSkeletonItem* item) const
{
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].item == item) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

QColor SceneManager::colorForIndex(int index)
{
    return kPalette[index % kPaletteSize];
}
