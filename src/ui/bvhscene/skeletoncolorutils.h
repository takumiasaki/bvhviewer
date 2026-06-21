#ifndef SKELETONCOLORUTILS_H
#define SKELETONCOLORUTILS_H

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

class QQmlEngine;
class QJSEngine;

class SkeletonColorUtils : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static SkeletonColorUtils* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    Q_INVOKABLE QColor colorFromTone(const QColor& jointColor, int tone) const;
    Q_INVOKABLE int defaultBoneTone() const;

private:
    explicit SkeletonColorUtils(QObject* parent = nullptr);
};

#endif // SKELETONCOLORUTILS_H
