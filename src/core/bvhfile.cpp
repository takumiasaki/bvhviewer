#include "bvhfile.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QVector3D>
#include <QDebug>
#include <QRegularExpression>

BvhFile::BvhFile()
    : rootNode(nullptr), totalChannels(0), currentFrame(-1), valid(false)
{
}

BvhFile::~BvhFile()
{
    delete rootNode;
}

bool BvhFile::load(const QString& filePath)
{
    if (rootNode) {
        delete rootNode;
        rootNode = nullptr;
    }
    valid = false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open BVH file:" << filePath;
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    QStringList lines = content.split('\n', Qt::SkipEmptyParts);

    // HIERARCHY と MOTION ブロックを探す
    int hierarchyIndex = -1;
    int motionIndex = -1;

    for (int i = 0; i < lines.size(); ++i) {
        QString trimmed = lines[i].trimmed();
        if (trimmed == "HIERARCHY") {
            hierarchyIndex = i;
        } else if (trimmed == "MOTION") {
            motionIndex = i;
        }
    }

    if (hierarchyIndex == -1) {
        qWarning() << "HIERARCHY block not found";
        return false;
    }

    // HIERARCHY ブロック解析
    int lineIdx = hierarchyIndex + 1;
    rootNode = parseBvhHierarchy(lines, lineIdx);
    if (!rootNode) {
        qWarning() << "Failed to parse HIERARCHY block";
        return false;
    }

    // ノードをフラット化してチャンネルインデックスを割り当て
    buildNodeList();
    assignChannelIndices();

    // MOTION ブロック解析
    if (motionIndex != -1) {
        lineIdx = motionIndex + 1;
        parseBvhMotion(lines, lineIdx);
    }

    valid = true;
    qDebug() << "BVH file loaded successfully:" << filePath;
    qDebug() << "Total nodes:" << flatNodeList.size();
    qDebug() << "Total channels:" << totalChannels;
    qDebug() << "Frames:" << motion.getFrameCount();

    return true;
}

BvhNode* BvhFile::parseBvhHierarchy(const QStringList& lines, int& lineIndex)
{
    BvhNode* node = nullptr;

    while (lineIndex < lines.size()) {
        QString line = lines[lineIndex].trimmed();
        ++lineIndex;

        if (line.isEmpty()) {
            continue;
        }

        QStringList tokens = line.split(' ', Qt::SkipEmptyParts);
        if (tokens.isEmpty()) {
            continue;
        }

        if (tokens[0] == "ROOT") {
            if (tokens.size() < 2) {
                qWarning() << "Invalid ROOT line";
                return nullptr;
            }
            node = new BvhNode(tokens[1]);
        } else if (tokens[0] == "JOINT") {
            if (tokens.size() < 2) {
                qWarning() << "Invalid JOINT line";
                return nullptr;
            }
            node = new BvhNode(tokens[1]);
        } else if (tokens[0] == "OFFSET") {
            if (tokens.size() < 4) {
                qWarning() << "Invalid OFFSET line";
                return nullptr;
            }
            float x = tokens[1].toFloat();
            float y = tokens[2].toFloat();
            float z = tokens[3].toFloat();
            if (node) {
                node->setOffset(QVector3D(x, y, z));
            }
        } else if (tokens[0] == "CHANNELS") {
            if (tokens.size() < 2) {
                qWarning() << "Invalid CHANNELS line";
                return nullptr;
            }
            int channelCount = tokens[1].toInt();
            if (tokens.size() < 2 + channelCount) {
                qWarning() << "Invalid CHANNELS line: not enough channel names";
                return nullptr;
            }
            for (int i = 0; i < channelCount; ++i) {
                Channel ch(tokens[2 + i]);
                if (node) {
                    node->addChannel(ch);
                }
            }
        } else if (tokens[0] == "{") {
            // 子ノード群を再帰的に解析
            while (lineIndex < lines.size()) {
                QString nextLine = lines[lineIndex].trimmed();
                if (nextLine.isEmpty()) {
                    ++lineIndex;
                    continue;
                }

                if (nextLine == "}") {
                    ++lineIndex;
                    break;
                }

                QStringList nextTokens = nextLine.split(' ', Qt::SkipEmptyParts);
                if (nextTokens.isEmpty()) {
                    ++lineIndex;
                    continue;
                }

                if (nextTokens[0] == "JOINT") {
                    BvhNode* child = parseBvhHierarchy(lines, lineIndex);
                    if (child && node) {
                        node->addChild(child);
                    }
                } else if (nextLine.startsWith("End Site")) {
                    ++lineIndex;
                    // End Site ブロック
                    while (lineIndex < lines.size()) {
                        QString siteLine = lines[lineIndex].trimmed();
                        ++lineIndex;

                        if (siteLine == "{") {
                            continue;
                        } else if (siteLine == "}") {
                            break;
                        }

                        QStringList siteTokens = siteLine.split(' ', Qt::SkipEmptyParts);
                        if (!siteTokens.isEmpty() && siteTokens[0] == "OFFSET") {
                            if (siteTokens.size() >= 4 && node) {
                                float x = siteTokens[1].toFloat();
                                float y = siteTokens[2].toFloat();
                                float z = siteTokens[3].toFloat();
                                node->setEndSite(QVector3D(x, y, z));
                            }
                        }
                    }
                } else if (nextTokens[0] == "OFFSET") {
                    if (nextTokens.size() >= 4 && node) {
                        float x = nextTokens[1].toFloat();
                        float y = nextTokens[2].toFloat();
                        float z = nextTokens[3].toFloat();
                        node->setOffset(QVector3D(x, y, z));
                    }
                    ++lineIndex;
                } else if (nextTokens[0] == "CHANNELS") {
                    int channelCount = 0;
                    if (nextTokens.size() >= 2) {
                        channelCount = nextTokens[1].toInt();
                    }

                    for (int i = 0; i < channelCount && 2 + i < nextTokens.size(); ++i) {
                        Channel ch(nextTokens[2 + i]);
                        if (node) {
                            node->addChannel(ch);
                        }
                    }

                    ++lineIndex;
                } else {
                    ++lineIndex;
                }
            }
            break;
        }
    }

    return node;
}

void BvhFile::parseBvhMotion(const QStringList& lines, int& lineIndex)
{
    int frameCount = 0;
    double frameTime = 0.0;

    // Frames と Frame Time を読み込み
    while (lineIndex < lines.size()) {
        QString line = lines[lineIndex].trimmed();
        ++lineIndex;

        if (line.isEmpty()) {
            continue;
        }

        QStringList tokens = line.split(' ', Qt::SkipEmptyParts);
        if (tokens.isEmpty()) {
            continue;
        }

        if (tokens[0] == "Frames:") {
            if (tokens.size() >= 2) {
                frameCount = tokens[1].toInt();
            }
        } else if (tokens[0] == "Frame") {
            // "Frame Time:" の場合
            if (tokens.size() >= 3 && tokens[1] == "Time:") {
                frameTime = tokens[2].toDouble();
            }
        } else {
            // フレームデータの開始
            // このトークンをフレームとして扱う
            --lineIndex;
            break;
        }
    }

    motion.setFrameCount(frameCount);
    motion.setFrameTime(frameTime);
    motion.setTotalChannels(totalChannels);
    motion.allocateFrames();

    // フレームデータを読み込み
    for (int f = 0; f < frameCount && lineIndex < lines.size(); ++f) {
        QString line = lines[lineIndex].trimmed();
        ++lineIndex;

        if (line.isEmpty()) {
            --f;  // スキップ
            continue;
        }

        QStringList tokens = line.split(' ', Qt::SkipEmptyParts);
        QVector<double> frameData;

        for (const QString& token : tokens) {
            bool ok;
            double value = token.toDouble(&ok);
            if (ok) {
                frameData.append(value);
            }
        }

        if (frameData.size() == totalChannels) {
            motion.setFrame(f, frameData);
        }
    }

    qDebug() << "Motion loaded: frames=" << frameCount << ", frame time=" << frameTime;
}

void BvhFile::buildNodeList()
{
    flatNodeList.clear();
    if (rootNode) {
        buildNodeListDFS(rootNode);
    }
}

void BvhFile::buildNodeListDFS(BvhNode* node)
{
    if (!node) {
        return;
    }

    flatNodeList.append(node);

    for (BvhNode* child : node->getChildren()) {
        buildNodeListDFS(child);
    }
}

void BvhFile::assignChannelIndices()
{
    totalChannels = 0;

    for (BvhNode* node : flatNodeList) {
        node->setChannelStartIndex(totalChannels);
        totalChannels += node->getChannelCount();
    }
}

void BvhFile::setCurrentFrame(int frameIndex)
{
    if (frameIndex < 0 || frameIndex >= motion.getFrameCount()) {
        qWarning() << "Invalid frame index:" << frameIndex;
        return;
    }

    currentFrame = frameIndex;
    QVector<double> frameData = motion.getFrame(frameIndex);

    // 各ノードにチャンネル値を設定
    for (BvhNode* node : flatNodeList) {
        int startIdx = node->getChannelStartIndex();
        int channelCount = node->getChannelCount();

        for (int i = 0; i < channelCount; ++i) {
            if (startIdx + i < frameData.size()) {
                node->setChannelValue(i, frameData[startIdx + i]);
            }
        }

        // ローカル変換を更新
        node->updateLocalTransform();
    }

    // ワールド座標系変換を更新（DFS順）
    for (BvhNode* node : flatNodeList) {
        BvhNode* parent = node->getParent();
        if (parent) {
            QMatrix4x4 worldTransform = parent->getWorldTransform() * node->getLocalTransform();
            node->setWorldTransform(worldTransform);
        } else {
            // ルートノード
            node->setWorldTransform(node->getLocalTransform());
        }
    }
}

BvhNode* BvhFile::getNodeByName(const QString& name) const
{
    for (BvhNode* node : flatNodeList) {
        if (node->getName() == name) {
            return node;
        }
    }
    return nullptr;
}

bool BvhFile::isValid() const
{
    return valid && rootNode != nullptr;
}

void BvhFile::printHierarchy() const
{
    if (!rootNode) {
        qDebug() << "No root node";
        return;
    }

    qDebug() << "BVH Hierarchy:";
    qDebug() << "Root:" << rootNode->getName();
    qDebug() << "Total nodes:" << flatNodeList.size();
    qDebug() << "Total channels:" << totalChannels;

    for (BvhNode* node : flatNodeList) {
        qDebug() << " -" << node->getName() << "channels:" << node->getChannelCount()
                 << "start index:" << node->getChannelStartIndex();
    }
}
