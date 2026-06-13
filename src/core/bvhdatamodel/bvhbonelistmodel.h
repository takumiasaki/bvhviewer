#ifndef BVHBONELISTMODEL_H
#define BVHBONELISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

struct Bvh3DBone;

class BvhBoneListModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum BoneRoles {
        BoneIndexRole = Qt::UserRole + 1,
        ParentJointIndexRole,
        ChildJointIndexRole,
        PositionRole,
        RotationRole,
        LengthRole
    };
    Q_ENUM(BoneRoles)

    explicit BvhBoneListModel(QObject* parent = nullptr);

    void setBones(const QVector<Bvh3DBone>* bones);
    void refreshAll();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    const QVector<Bvh3DBone>* m_bones = nullptr;
};

#endif // BVHBONELISTMODEL_H
