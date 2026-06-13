#ifndef BVHSCENE_FOREIGN_TYPES_H
#define BVHSCENE_FOREIGN_TYPES_H

#include <QtQml/qqmlregistration.h>

#include "bvh3dmodel.h"
#include "bvhbonelistmodel.h"
#include "bvhjointlistmodel.h"

struct Bvh3DModelForeign {
    Q_GADGET
    QML_FOREIGN(Bvh3DModel)
    QML_NAMED_ELEMENT(Bvh3DModel)
    QML_UNCREATABLE("Bvh3DModel instances are owned by SceneManager")
};

struct BvhJointListModelForeign {
    Q_GADGET
    QML_FOREIGN(BvhJointListModel)
    QML_NAMED_ELEMENT(BvhJointListModel)
    QML_UNCREATABLE("BvhJointListModel is exposed via BvhSkeletonItem")
};

struct BvhBoneListModelForeign {
    Q_GADGET
    QML_FOREIGN(BvhBoneListModel)
    QML_NAMED_ELEMENT(BvhBoneListModel)
    QML_UNCREATABLE("BvhBoneListModel is exposed via BvhSkeletonItem")
};

#endif // BVHSCENE_FOREIGN_TYPES_H
