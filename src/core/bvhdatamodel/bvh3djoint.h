#ifndef BVH3DJOINT_H
#define BVH3DJOINT_H

#include <QString>
#include <QVector3D>

struct Bvh3DJoint {
    int index = 0;
    int bvhNodeIndex = 0;
    QString name;
    bool isEndSite = false;
    QVector3D position;
};

#endif // BVH3DJOINT_H
