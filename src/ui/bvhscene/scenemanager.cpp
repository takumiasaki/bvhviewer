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
constexpr int kDefaultBoneTone = -25;

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
    emit activeIndexChanged();
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
    const double ft = frameTime();
    if (ft <= 0.0) {
        return 0;
    }

    const double d = duration();
    return static_cast<int>(std::floor(d / ft)) + 1;
}

double SceneManager::frameTime() const
{
    double minFrameTime = 0.0;
    bool found = false;

    for (const SkeletonEntry& entry : m_entries) {
        if (!entry.item || !entry.item->isValid() || entry.item->frameCount() <= 0) {
            continue;
        }

        const double ft = entry.item->frameTime();
        if (ft <= 0.0) {
            continue;
        }

        if (!found || ft < minFrameTime) {
            minFrameTime = ft;
            found = true;
        }
    }

    return found ? minFrameTime : 0.0;
}

qreal SceneManager::duration() const
{
    double maxDuration = 0.0;

    for (const SkeletonEntry& entry : m_entries) {
        if (!entry.item || !entry.item->isValid()) {
            continue;
        }

        const int count = entry.item->frameCount();
        if (count <= 1) {
            continue;
        }

        const double skeletonDuration = static_cast<double>(count - 1) * entry.item->frameTime();
        maxDuration = qMax(maxDuration, skeletonDuration);
    }

    return maxDuration;
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
    const int paletteIndex = static_cast<int>(m_entries.size());
    const QColor jointColor = defaultColorForPaletteIndex(paletteIndex);
    model->setBoneTone(kDefaultBoneTone);
    model->setJointColor(jointColor);
    model->setCustomBoneColor(Bvh3DModel::colorFromTone(jointColor, kDefaultBoneTone));
    model->setBoneColorMode(Bvh3DModel::ToneOffset);

    const int insertIndex = paletteIndex;
    beginInsertRows(QModelIndex(), insertIndex, insertIndex);

    SkeletonEntry entry;
    entry.bvhFile = std::move(bvhFile);
    entry.model = std::move(model);
    entry.sourcePath = filePath;
    entry.paletteIndex = paletteIndex;
    entry.item = new BvhSkeletonItem(entry.model.get(), filePath, paletteIndex, this);
    connectSkeletonSignals(entry.item);

    m_entries.push_back(std::move(entry));
    endInsertRows();

    relayoutSceneOffsets();

    emit sceneCountChanged();
    emitTimelineChanged();

    if (m_currentFrame < 0) {
        setAnimationTime(0.0);
    } else {
        clampAnimationTimeToDuration();
        applyAnimationTimeToVisible();
        syncCurrentFrameFromAnimationTime();
    }

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
        setPlaying(false);
        m_currentFrame = -1;
        m_animationTime = 0.0;
        emit currentFrameChanged();
        emit animationTimeChanged();
    } else if (m_activeIndex == index) {
        setActiveIndex(qMin(index, static_cast<int>(m_entries.size()) - 1));
    } else if (m_activeIndex > index) {
        m_activeIndex -= 1;
        emit activeIndexChanged();
    }

    relayoutSceneOffsets();
    emit sceneCountChanged();
    emitTimelineChanged();
    clampAnimationTimeToDuration();
    applyAnimationTimeToVisible();
    syncCurrentFrameFromAnimationTime();
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

void SceneManager::setFloorShadowsEnabled(bool enabled)
{
    if (m_floorShadowsEnabled == enabled) {
        return;
    }
    m_floorShadowsEnabled = enabled;
    emit floorShadowsEnabledChanged();
}

void SceneManager::setAnimationTime(qreal time)
{
    const qreal d = duration();
    const qreal clamped = d > 0.0 ? qBound<qreal>(0.0, time, d) : 0.0;
    if (qFuzzyCompare(clamped + 1.0, m_animationTime + 1.0)) {
        return;
    }

    m_animationTime = clamped;
    applyAnimationTimeToVisible();
    syncCurrentFrameFromAnimationTime();
    emit animationTimeChanged();
}

void SceneManager::setCurrentFrame(int frame)
{
    const double ft = frameTime();
    if (ft <= 0.0) {
        return;
    }

    const int maxFrame = qMax(0, frameCount() - 1);
    const int clampedFrame = qBound(0, frame, maxFrame);
    setAnimationTime(static_cast<qreal>(clampedFrame) * ft);
}

void SceneManager::play()
{
    if (frameCount() <= 1 || duration() <= 0.0) {
        return;
    }

    if (m_animationTime >= duration()) {
        setAnimationTime(0.0);
    }

    setPlaying(true);
}

void SceneManager::pause()
{
    setPlaying(false);
}

void SceneManager::toggle()
{
    if (m_playing) {
        pause();
    } else {
        play();
    }
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

void SceneManager::applyAnimationTimeToVisible()
{
    for (const SkeletonEntry& entry : m_entries) {
        if (entry.item && entry.item->visible()) {
            syncPoseForSkeleton(entry.item);
        }
    }
}

void SceneManager::syncPoseForSkeleton(BvhSkeletonItem* item)
{
    if (!item || !item->visible()) {
        return;
    }

    Bvh3DModel* model = item->model();
    if (model && model->isValid()) {
        model->setPoseAtTime(m_animationTime);
    }
}

void SceneManager::syncCurrentFrameFromAnimationTime()
{
    const double ft = frameTime();
    if (ft <= 0.0) {
        if (m_currentFrame != -1) {
            m_currentFrame = -1;
            emit currentFrameChanged();
        }
        return;
    }

    int frame = qRound(m_animationTime / ft);
    const int maxFrame = qMax(0, frameCount() - 1);
    frame = qBound(0, frame, maxFrame);

    if (frame == m_currentFrame) {
        return;
    }

    m_currentFrame = frame;
    emit currentFrameChanged();
}

void SceneManager::emitTimelineChanged()
{
    emit frameCountChanged();
    emit frameTimeChanged();
    emit durationChanged();
}

void SceneManager::clampAnimationTimeToDuration()
{
    const qreal d = duration();
    const qreal clamped = d > 0.0 ? qBound<qreal>(0.0, m_animationTime, d) : 0.0;
    if (qFuzzyCompare(clamped + 1.0, m_animationTime + 1.0)) {
        return;
    }

    m_animationTime = clamped;
    emit animationTimeChanged();
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
        emitTimelineChanged();
        handleSkeletonUpdated();
    });
    connect(item, &BvhSkeletonItem::frameTimeChanged, this, [this]() {
        emitTimelineChanged();
        handleSkeletonUpdated();
    });
    connect(item, &BvhSkeletonItem::visibleChanged, this, [this, item]() {
        if (item->visible()) {
            syncPoseForSkeleton(item);
        }
    });
    connect(item, &BvhSkeletonItem::jointColorChanged, this, &SceneManager::handleSkeletonUpdated);
    connect(item, &BvhSkeletonItem::boneColorChanged, this, &SceneManager::handleSkeletonUpdated);
    connect(item, &BvhSkeletonItem::boneColorModeChanged, this, &SceneManager::handleSkeletonUpdated);
    connect(item, &BvhSkeletonItem::boneToneChanged, this, &SceneManager::handleSkeletonUpdated);
    connect(item, &BvhSkeletonItem::customBoneColorChanged, this, &SceneManager::handleSkeletonUpdated);
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

QColor SceneManager::defaultColorForPaletteIndex(int index)
{
    return kPalette[index % kPaletteSize];
}

QColor SceneManager::colorForIndex(int index) const
{
    return defaultColorForPaletteIndex(index);
}

void SceneManager::resetSkeletonColors(int index)
{
    if (index < 0 || index >= static_cast<int>(m_entries.size())) {
        return;
    }

    BvhSkeletonItem* item = m_entries.at(static_cast<size_t>(index)).item;
    if (!item) {
        return;
    }

    const QColor jointColor = defaultColorForPaletteIndex(m_entries.at(static_cast<size_t>(index)).paletteIndex);
    item->setBoneTone(kDefaultBoneTone);
    item->setCustomBoneColor(Bvh3DModel::colorFromTone(jointColor, kDefaultBoneTone));
    item->setJointColor(jointColor);
    item->setBoneColorMode(Bvh3DModel::ToneOffset);
}
