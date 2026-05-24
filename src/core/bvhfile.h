#ifndef BVHFILE_H
#define BVHFILE_H

#include <QString>
#include <QList>
#include "bvhnode.h"
#include "bvhmotion.h"

class BvhFile {
public:
    BvhFile();
    ~BvhFile();

    // ファイル読み込み
    bool load(const QString& filePath);
    bool isValid() const;

    // ルート情報
    BvhNode* getRootNode() const { return rootNode; }

    // フレーム操作
    void setCurrentFrame(int frameIndex);
    int getFrameCount() const { return motion.getFrameCount(); }
    double getFrameTime() const { return motion.getFrameTime(); }
    double getTotalDuration() const { return motion.getTotalDuration(); }
    int getCurrentFrame() const { return currentFrame; }

    // ノード検索
    BvhNode* getNodeByName(const QString& name) const;
    const QList<BvhNode*>& getAllNodes() const { return flatNodeList; }
    int getTotalChannels() const { return totalChannels; }

    // デバッグ出力
    void printHierarchy() const;

private:
    // 階層情報
    BvhNode* rootNode;

    // モーション情報
    BvhMotion motion;

    // 最適化用キャッシュ
    QList<BvhNode*> flatNodeList;  // DFS順にフラット化したノード
    int totalChannels;
    int currentFrame;

    bool valid;

    // パース用内部メソッド
    BvhNode* parseBvhHierarchy(const QStringList& lines, int& lineIndex);
    void parseBvhMotion(const QStringList& lines, int& lineIndex);
    void buildNodeList();
    void buildNodeListDFS(BvhNode* node);
    void assignChannelIndices();
};

#endif // BVHFILE_H
