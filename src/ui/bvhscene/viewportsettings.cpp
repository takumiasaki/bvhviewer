#include "viewportsettings.h"

namespace {

constexpr char kDefaultGroundColor[] = "#50505a";

} // namespace

ViewportSettings::ViewportSettings(QObject* parent)
    : QObject(parent)
    , m_groundColor(defaultGroundColor())
{
}

QColor ViewportSettings::defaultGroundColor()
{
    return QColor(QString::fromLatin1(kDefaultGroundColor));
}

void ViewportSettings::setGroundColor(const QColor& color)
{
    if (!color.isValid() || m_groundColor == color) {
        return;
    }
    m_groundColor = color;
    emit groundColorChanged();
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
}
