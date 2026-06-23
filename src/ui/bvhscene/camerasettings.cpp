#include "camerasettings.h"

CameraSettings::CameraSettings(QObject* parent)
    : QObject(parent)
{
}

void CameraSettings::setMode(CameraMode mode)
{
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    emit modeChanged();
}

void CameraSettings::requestFrameNow(bool instant)
{
    emit frameNowRequested(instant);
}

void CameraSettings::requestResetView()
{
    emit resetViewRequested();
}
