#ifndef CAMERASETTINGS_H
#define CAMERASETTINGS_H

#include <QObject>

#include <QtQml/qqmlregistration.h>

class CameraSettings : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(CameraMode mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum CameraMode {
        Manual = 0,
        Auto = 1
    };
    Q_ENUM(CameraMode)

    explicit CameraSettings(QObject* parent = nullptr);

    CameraMode mode() const { return m_mode; }
    void setMode(CameraMode mode);

    Q_INVOKABLE void requestFrameNow(bool instant = false);
    Q_INVOKABLE void requestResetView();

signals:
    void modeChanged();
    void frameNowRequested(bool instant);
    void resetViewRequested();

private:
    CameraMode m_mode = Auto;
};

#endif // CAMERASETTINGS_H
