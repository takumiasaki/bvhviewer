#include "skeletoncolorutils.h"

#include "bvh3dmodel.h"

#include <QQmlEngine>
#include <QJSEngine>

SkeletonColorUtils::SkeletonColorUtils(QObject* parent)
    : QObject(parent)
{
}

SkeletonColorUtils* SkeletonColorUtils::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    static SkeletonColorUtils instance;
    return &instance;
}

QColor SkeletonColorUtils::colorFromTone(const QColor& jointColor, int tone) const
{
    return Bvh3DModel::colorFromTone(jointColor, tone);
}

int SkeletonColorUtils::defaultBoneTone() const
{
    return Bvh3DModel::defaultBoneTone();
}
