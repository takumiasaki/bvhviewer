#ifndef BVH3DBONE_H
#define BVH3DBONE_H

#include <QQuaternion>
#include <QVector3D>

struct Bvh3DBone {
    int index = 0;
    int parentJointIndex = 0;
    int childJointIndex = 0;
    QVector3D position;
    QQuaternion rotation;
    float length = 0.0f;
};

#endif // BVH3DBONE_H
