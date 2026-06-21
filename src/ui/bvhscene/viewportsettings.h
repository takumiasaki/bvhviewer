#ifndef VIEWPORTSETTINGS_H
#define VIEWPORTSETTINGS_H

#include <QColor>
#include <QObject>

#include <QtQml/qqmlregistration.h>

class ViewportSettings : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QColor groundColor READ groundColor WRITE setGroundColor NOTIFY groundColorChanged)
    Q_PROPERTY(bool floorShadowsEnabled READ floorShadowsEnabled WRITE setFloorShadowsEnabled NOTIFY floorShadowsEnabledChanged)

public:
    explicit ViewportSettings(QObject* parent = nullptr);

    QColor groundColor() const { return m_groundColor; }
    void setGroundColor(const QColor& color);

    bool floorShadowsEnabled() const { return m_floorShadowsEnabled; }
    void setFloorShadowsEnabled(bool enabled);

    Q_INVOKABLE void resetGroundToDefault();

signals:
    void groundColorChanged();
    void floorShadowsEnabledChanged();

private:
    static QColor defaultGroundColor();

    QColor m_groundColor;
    bool m_floorShadowsEnabled = true;
};

#endif // VIEWPORTSETTINGS_H
