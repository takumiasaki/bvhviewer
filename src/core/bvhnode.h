#ifndef BVHNODE_H
#define BVHNODE_H

#include <QString>
#include <QVector>
#include <QList>
#include <QVector3D>
#include <QMatrix4x4>
#include "channel.h"

class BvhNode {
public:
    // コンストラクタ・デストラクタ
    BvhNode(const QString& name = "", BvhNode* parent = nullptr);
    ~BvhNode();

    // 階層情報
    QString getName() const { return name; }
    BvhNode* getParent() const { return parent; }
    const QList<BvhNode*>& getChildren() const { return children; }
    void addChild(BvhNode* child);

    // ジオメトリ情報
    void setOffset(const QVector3D& offset) { this->offset = offset; }
    QVector3D getOffset() const { return offset; }

    void addChannel(const Channel& channel) { channels.append(channel); }
    const QList<Channel>& getChannels() const { return channels; }
    int getChannelCount() const { return channels.size(); }

    void setChannelStartIndex(int index) { channelStartIndex = index; }
    int getChannelStartIndex() const { return channelStartIndex; }

    void setChannelValue(int channelIndex, double value);
    double getChannelValue(int channelIndex) const;

    // End Site 情報
    void setEndSite(const QVector3D& offset);
    bool hasEndSiteInfo() const { return hasEndSite; }
    QVector3D getEndSiteOffset() const { return endSiteOffset; }

    // 変換情報
    void updateLocalTransform();
    QMatrix4x4 getLocalTransform() const { return localTransform; }
    QMatrix4x4 getWorldTransform() const { return worldTransform; }

    void setWorldTransform(const QMatrix4x4& transform) { worldTransform = transform; }

    // ワールド座標系での位置取得
    QVector3D getWorldPosition() const;

private:
    // 階層情報
    QString name;
    BvhNode* parent;
    QList<BvhNode*> children;

    // ジオメトリ情報
    QVector3D offset;
    QList<Channel> channels;
    int channelStartIndex;

    // End Site 情報
    bool hasEndSite;
    QVector3D endSiteOffset;

    // 変換情報（計算結果キャッシュ）
    QMatrix4x4 localTransform;
    QMatrix4x4 worldTransform;

    // 内部ヘルパー
    QMatrix4x4 createRotationMatrix() const;
    QVector3D extractPosition() const;
};

#endif // BVHNODE_H
