#include "viewportsettings.h"

namespace {

constexpr char kDefaultGroundColor[] = "#50505a";
constexpr qreal kMinGroundSize = 1.0;

bool isValidGroundSize(qreal size)
{
    return size >= kMinGroundSize;
}

} // namespace

ViewportSettings::ViewportSettings(QObject* parent)
    : QObject(parent)
    , m_groundColor(defaultGroundColor())
    , m_groundShape(defaultGroundShape())
    , m_groundSizeX(defaultGroundSizeX())
    , m_groundSizeY(defaultGroundSizeY())
    , m_groundSizeZ(defaultGroundSizeZ())
{
}

QColor ViewportSettings::defaultGroundColor()
{
    return QColor(QString::fromLatin1(kDefaultGroundColor));
}

ViewportSettings::GroundShape ViewportSettings::defaultGroundShape()
{
    return Rectangle;
}

qreal ViewportSettings::defaultGroundSizeX()
{
    return 2000.0;
}

qreal ViewportSettings::defaultGroundSizeY()
{
    return 1.0;
}

qreal ViewportSettings::defaultGroundSizeZ()
{
    return 2000.0;
}

void ViewportSettings::setGroundColor(const QColor& color)
{
    if (!color.isValid() || m_groundColor == color) {
        return;
    }
    m_groundColor = color;
    emit groundColorChanged();
}

void ViewportSettings::setGroundShape(GroundShape shape)
{
    if (m_groundShape == shape) {
        return;
    }
    m_groundShape = shape;
    emit groundShapeChanged();
}

void ViewportSettings::setGroundSizeX(qreal size)
{
    if (!isValidGroundSize(size) || qFuzzyCompare(m_groundSizeX, size)) {
        return;
    }
    m_groundSizeX = size;
    emit groundSizeXChanged();
}

void ViewportSettings::setGroundSizeY(qreal size)
{
    if (!isValidGroundSize(size) || qFuzzyCompare(m_groundSizeY, size)) {
        return;
    }
    m_groundSizeY = size;
    emit groundSizeYChanged();
}

void ViewportSettings::setGroundSizeZ(qreal size)
{
    if (!isValidGroundSize(size) || qFuzzyCompare(m_groundSizeZ, size)) {
        return;
    }
    m_groundSizeZ = size;
    emit groundSizeZChanged();
}

void ViewportSettings::setFloorShadowsEnabled(bool enabled)
{
    if (m_floorShadowsEnabled == enabled) {
        return;
    }
    m_floorShadowsEnabled = enabled;
    emit floorShadowsEnabledChanged();
}

void ViewportSettings::resetGroundToDefault()
{
    setGroundColor(defaultGroundColor());
    setGroundShape(defaultGroundShape());
    setGroundSizeX(defaultGroundSizeX());
    setGroundSizeY(defaultGroundSizeY());
    setGroundSizeZ(defaultGroundSizeZ());
}
