#ifndef BVHJOINTLISTMODEL_H
#define BVHJOINTLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct Bvh3DJoint;

class BvhJointListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum JointRoles {
        JointIndexRole = Qt::UserRole + 1,
        NameRole,
        PositionRole,
        IsEndSiteRole
    };
    Q_ENUM(JointRoles)

    explicit BvhJointListModel(QObject* parent = nullptr);

    void setJoints(const QVector<Bvh3DJoint>* joints);
    void refreshAll();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    const QVector<Bvh3DJoint>* m_joints = nullptr;
};

#endif // BVHJOINTLISTMODEL_H
