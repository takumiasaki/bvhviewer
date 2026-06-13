#include "bvhjointlistmodel.h"
#include "bvh3djoint.h"

BvhJointListModel::BvhJointListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

void BvhJointListModel::setJoints(const QVector<Bvh3DJoint>* joints)
{
    beginResetModel();
    m_joints = joints;
    endResetModel();
}

void BvhJointListModel::refreshAll()
{
    if (!m_joints || m_joints->isEmpty()) {
        return;
    }

    const QModelIndex topLeft = index(0, 0);
    const QModelIndex bottomRight = index(m_joints->size() - 1, 0);
    emit dataChanged(topLeft, bottomRight, {JointIndexRole, NameRole, PositionRole, IsEndSiteRole});
}

int BvhJointListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_joints) {
        return 0;
    }
    return m_joints->size();
}

QVariant BvhJointListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_joints || index.row() < 0 || index.row() >= m_joints->size()) {
        return {};
    }

    const Bvh3DJoint& joint = m_joints->at(index.row());
    switch (role) {
    case JointIndexRole:
        return joint.index;
    case NameRole:
        return joint.name;
    case PositionRole:
        return joint.position;
    case IsEndSiteRole:
        return joint.isEndSite;
    default:
        return {};
    }
}

QHash<int, QByteArray> BvhJointListModel::roleNames() const
{
    return {
        {JointIndexRole, "jointIndex"},
        {NameRole, "name"},
        {PositionRole, "position"},
        {IsEndSiteRole, "isEndSite"},
    };
}
