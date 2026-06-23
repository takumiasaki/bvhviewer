#ifndef POSE_UTILS_H
#define POSE_UTILS_H

#include "bvh3djoint.h"

#include <QVector3D>
#include <QQuaternion>
#include <QVector>

class BvhFile;

namespace PoseUtils {

struct BoneTransform {
    QVector3D position;
    QQuaternion rotation;
    float length = 0.0f;
};

struct PoseBounds {
    QVector3D min;
    QVector3D max;
    bool valid = false;
};

constexpr float kDefaultFramingMargin = 1.18f;

PoseBounds computePoseBounds(const QVector<Bvh3DJoint>& joints,
                             float jointRadius,
                             float endSiteRadius,
                             float marginFactor = kDefaultFramingMargin);
PoseBounds mergePoseBounds(const PoseBounds& a, const PoseBounds& b);
QVector3D boundsCenter(const PoseBounds& bounds);
float framingRadius(const PoseBounds& bounds);
float framingDistance(const PoseBounds& bounds,
                      float fovDegrees,
                      float aspectRatio,
                      float pitchDegrees = 0.0f);

BoneTransform computeBoneTransform(const QVector3D& parentPosition,
                                   const QVector3D& childPosition);

void applyChannelValues(BvhFile* bvhFile, const QVector<double>& frameData);

void applyInterpolatedFrame(BvhFile* bvhFile, double frameReal);

} // namespace PoseUtils

#endif // POSE_UTILS_H
