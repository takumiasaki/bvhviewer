#include "bvhbonelistmodel.h"
#include "bvh3dbone.h"

BvhBoneListModel::BvhBoneListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void BvhBoneListModel::setBones(const QVector<Bvh3DBone>* bones)
{
    beginResetModel();
    m_bones = bones;
    endResetModel();
}

void BvhBoneListModel::refreshAll()
{
    if (!m_bones || m_bones->isEmpty()) {
        return;
    }

    const QModelIndex topLeft = index(0, 0);
    const QModelIndex bottomRight = index(m_bones->size() - 1, 0);
    emit dataChanged(topLeft, bottomRight,
                     {BoneIndexRole, ParentJointIndexRole, ChildJointIndexRole,
                      PositionRole, RotationRole, LengthRole});
}

int BvhBoneListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_bones) {
        return 0;
    }
    return m_bones->size();
}

QVariant BvhBoneListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_bones || index.row() < 0 || index.row() >= m_bones->size()) {
        return {};
    }

    const Bvh3DBone& bone = m_bones->at(index.row());
    switch (role) {
    case BoneIndexRole:
        return bone.index;
    case ParentJointIndexRole:
        return bone.parentJointIndex;
    case ChildJointIndexRole:
        return bone.childJointIndex;
    case PositionRole:
        return bone.position;
    case RotationRole:
        return bone.rotation;
    case LengthRole:
        return bone.length;
    default:
        return {};
    }
}

QHash<int, QByteArray> BvhBoneListModel::roleNames() const
{
    return {
        {BoneIndexRole, "boneIndex"},
        {ParentJointIndexRole, "parentJointIndex"},
        {ChildJointIndexRole, "childJointIndex"},
        {PositionRole, "position"},
        {RotationRole, "rotation"},
        {LengthRole, "length"},
    };
}
