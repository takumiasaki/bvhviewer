#ifndef POSE_UTILS_H
#define POSE_UTILS_H

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

BoneTransform computeBoneTransform(const QVector3D& parentPosition,
                                   const QVector3D& childPosition);

void applyChannelValues(BvhFile* bvhFile, const QVector<double>& frameData);

void applyInterpolatedFrame(BvhFile* bvhFile, double frameReal);

} // namespace PoseUtils

#endif // POSE_UTILS_H
