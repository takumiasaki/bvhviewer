#include "bvhnode.h"
#include <QMatrix4x4>
#include <cmath>

BvhNode::BvhNode(const QString& name, BvhNode* parent)
    : name(name), parent(parent), channelStartIndex(0), hasEndSite(false), 
      endSiteOffset(0, 0, 0)
{
    localTransform.setToIdentity();
    worldTransform.setToIdentity();
}

BvhNode::~BvhNode()
{
    // 子ノードは親が削除時に削除
    qDeleteAll(children);
}

void BvhNode::addChild(BvhNode* child)
{
    if (child) {
        child->parent = this;
        children.append(child);
    }
}

void BvhNode::setChannelValue(int channelIndex, double value)
{
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        channels[channelIndex].value = value;
    }
}

double BvhNode::getChannelValue(int channelIndex) const
{
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        return channels[channelIndex].value;
    }
    return 0.0;
}

void BvhNode::setEndSite(const QVector3D& offset)
{
    hasEndSite = true;
    endSiteOffset = offset;
}

QVector3D BvhNode::extractPosition() const
{
    QVector3D pos(0, 0, 0);

    for (const Channel& ch : channels) {
        switch (ch.type) {
        case Channel::Xposition:
            pos.setX(ch.value);
            break;
        case Channel::Yposition:
            pos.setY(ch.value);
            break;
        case Channel::Zposition:
            pos.setZ(ch.value);
            break;
        default:
            break;
        }
    }

    return pos;
}

QMatrix4x4 BvhNode::createRotationMatrix() const
{
    QMatrix4x4 rotMatrix;
    rotMatrix.setToIdentity();

    // チャンネル順に回転を適用（度をラジアンに変換）
    for (const Channel& ch : channels) {
        double angleRad = ch.value * M_PI / 180.0;

        switch (ch.type) {
        case Channel::Xrotation: {
            QMatrix4x4 rx;
            rx.setToIdentity();
            rx.rotate(ch.value, 1, 0, 0);
            rotMatrix = rotMatrix * rx;
            break;
        }
        case Channel::Yrotation: {
            QMatrix4x4 ry;
            ry.setToIdentity();
            ry.rotate(ch.value, 0, 1, 0);
            rotMatrix = rotMatrix * ry;
            break;
        }
        case Channel::Zrotation: {
            QMatrix4x4 rz;
            rz.setToIdentity();
            rz.rotate(ch.value, 0, 0, 1);
            rotMatrix = rotMatrix * rz;
            break;
        }
        default:
            break;
        }
    }

    return rotMatrix;
}

void BvhNode::updateLocalTransform()
{
    localTransform.setToIdentity();

    // オフセットと位置を組み合わせ
    QVector3D position = offset + extractPosition();

    // 回転行列を計算
    QMatrix4x4 rotMatrix = createRotationMatrix();

    // 変換を合成: Translation × Rotation
    localTransform.translate(position);
    localTransform = localTransform * rotMatrix;
}

QVector3D BvhNode::getWorldPosition() const
{
    return worldTransform.column(3).toVector3D();
}
