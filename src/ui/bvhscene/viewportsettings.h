#ifndef VIEWPORTSETTINGS_H
#define VIEWPORTSETTINGS_H

#include <QColor>
#include <QObject>

#include <QtQml/qqmlregistration.h>

class ViewportSettings : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QColor groundColor READ groundColor WRITE setGroundColor NOTIFY groundColorChanged)
    Q_PROPERTY(GroundShape groundShape READ groundShape WRITE setGroundShape NOTIFY groundShapeChanged)
    Q_PROPERTY(qreal groundSizeX READ groundSizeX WRITE setGroundSizeX NOTIFY groundSizeXChanged)
    Q_PROPERTY(qreal groundSizeY READ groundSizeY WRITE setGroundSizeY NOTIFY groundSizeYChanged)
    Q_PROPERTY(qreal groundSizeZ READ groundSizeZ WRITE setGroundSizeZ NOTIFY groundSizeZChanged)
    Q_PROPERTY(bool floorShadowsEnabled READ floorShadowsEnabled WRITE setFloorShadowsEnabled NOTIFY floorShadowsEnabledChanged)

public:
    enum GroundShape {
        None = 0,
        Rectangle = 1,
        Ellipse = 2
    };
    Q_ENUM(GroundShape)

    explicit ViewportSettings(QObject* parent = nullptr);

    QColor groundColor() const { return m_groundColor; }
    void setGroundColor(const QColor& color);

    GroundShape groundShape() const { return m_groundShape; }
    void setGroundShape(GroundShape shape);

    qreal groundSizeX() const { return m_groundSizeX; }
    void setGroundSizeX(qreal size);

    qreal groundSizeY() const { return m_groundSizeY; }
    void setGroundSizeY(qreal size);

    qreal groundSizeZ() const { return m_groundSizeZ; }
    void setGroundSizeZ(qreal size);

    bool floorShadowsEnabled() const { return m_floorShadowsEnabled; }
    void setFloorShadowsEnabled(bool enabled);

    Q_INVOKABLE void resetGroundToDefault();

signals:
    void groundColorChanged();
    void groundShapeChanged();
    void groundSizeXChanged();
    void groundSizeYChanged();
    void groundSizeZChanged();
    void floorShadowsEnabledChanged();

private:
    static QColor defaultGroundColor();
    static GroundShape defaultGroundShape();
    static qreal defaultGroundSizeX();
    static qreal defaultGroundSizeY();
    static qreal defaultGroundSizeZ();

    QColor m_groundColor;
    GroundShape m_groundShape = Rectangle;
    qreal m_groundSizeX = 2000.0;
    qreal m_groundSizeY = 1.0;
    qreal m_groundSizeZ = 2000.0;
    bool m_floorShadowsEnabled = true;
};

#endif // VIEWPORTSETTINGS_H
