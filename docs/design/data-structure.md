# BVH データ構造設計

## 概要

BVH ファイルをメモリ内に表現するための C++ クラス設計。モーションキャプチャデータの階層構造（スケルトン）とフレーム単位のモーションデータを効率的に管理し、Qt Quick での 3D 表示・アニメーション表示に対応することを想定している。

## アーキテクチャ

### 全体レイヤー構成

```
BvhFile （トップレベル、外部インターフェース）
  ├─ Hierarchy
  │  ├─ BvhNode (ROOT)
  │  │  ├─ BvhNode (JOINT)
  │  │  │  └─ BvhNode (JOINT) ...
  │  │  └─ BvhNode (JOINT) ...
  │  └─ flatNodeList （DFS順フラット化）
  │
  └─ Motion
     ├─ frameCount
     ├─ frameTime
     └─ frames （フレームデータ配列）
```

### クラス設計詳細

#### 1. Channel クラス

**目的**: BVH ファイルで定義されたチャンネル（Xposition, Yrotation 等）を表現

```cpp
class Channel {
public:
    enum Type {
        Xposition, Yposition, Zposition,
        Xrotation, Yrotation, Zrotation,
        Unknown
    };
    
    QString name;           // "Xposition", "Yrotation" など（ファイルから読み込み）
    Type type;              // Type列挙値
    double value;           // 現在フレームの値
};
```

**責務**:
- チャンネル名を Type 列挙値に変換
- 現在値の保持

---

#### 2. BvhNode クラス

**目的**: スケルトンの各ジョイント/ボーンを表現し、階層構造と変換情報を管理

**メンバー変数**:

```cpp
class BvhNode {
public:
    // 階層情報
    QString name;                          // ジョイント名
    BvhNode* parent;                       // 親ノード
    QList<BvhNode*> children;              // 子ノード群
    
    // ジオメトリ情報
    QVector3D offset;                      // 親からのローカルオフセット
    QList<Channel> channels;               // このノードが持つチャンネル
    int channelStartIndex;                 // BvhMotion::frames 内での開始インデックス
    
    // End Site 情報（leaf ノード用）
    bool hasEndSite;
    QVector3D endSiteOffset;
    
    // 変換情報（計算結果キャッシュ）
    QMatrix4x4 localTransform;             // ローカル座標系での変換
    QMatrix4x4 worldTransform;             // ワールド座標系での変換
    
private:
    // 内部メソッド
    void updateLocalTransformImpl();        // channels から localTransform を計算
};
```

**責務**:

- ジョイント階層の管理（親子関係）
- ローカル変換の計算
  - `channels` の値を使い、オフセットと回転を合成
  - 回転はチャンネル順に行列合成（Z→X→Y の順なら、その順で適用）
- 変換行列の格納（ローカル・ワールド座標系）

**public メソッド**:

```cpp
class BvhNode {
public:
    void setChannelValue(int channelIndex, double value);
    double getChannelValue(int channelIndex) const;
    
    // 変換計算
    void updateLocalTransform();
    // ※ ワールド座標系変換は BvhFile::setCurrentFrame() 内で一括実行
    
    // トラバーサル
    QList<BvhNode*> getChildren() const { return children; }
    BvhNode* getParent() const { return parent; }
    
    // 情報取得
    QMatrix4x4 getWorldTransform() const { return worldTransform; }
    QVector3D getWorldPosition() const { return worldTransform.column(3).toVector3D(); }
};
```

---

#### 3. BvhMotion クラス

**目的**: モーション フレームデータを効率的に保持・アクセス

```cpp
class BvhMotion {
public:
    int frameCount;                        // フレーム総数
    double frameTime;                      // 各フレーム間の時間（秒）
    
    QList<QVector<double>> frames;         // frames[i] = フレーム i の全チャンネル値配列
    int totalChannels;                     // 全チャンネル数
};
```

**責務**:

- モーション情報（フレーム数・フレームタイム）保持
- フレームデータの格納・効率的アクセス

**public メソッド**:

```cpp
class BvhMotion {
public:
    double getChannelValue(int frameIndex, int channelIndex) const;
    void setChannelValue(int frameIndex, int channelIndex, double value);
    
    QVector<double> getFrame(int frameIndex) const;
};
```

---

#### 4. BvhFile クラス

**目的**: BVH ファイルを読み込み、全データを統合管理。外部インターフェース

```cpp
class BvhFile {
private:
    // 階層情報
    BvhNode* rootNode;
    
    // モーション情報
    BvhMotion motion;
    
    // 最適化用キャッシュ
    QList<BvhNode*> flatNodeList;          // DFS順にフラット化したノード (高速走査用)
    int totalChannels;
    
    // パース用内部メソッド
    BvhNode* parseBvhHierarchy(const QStringList& lines, int& lineIndex);
    void parseBvhMotion(const QStringList& lines, int& lineIndex);
};
```

**責務**:

- BVH ファイルの読み込み・パース
- ノードツリー構築
- チャンネルレイアウト構築（channelStartIndex の計算）
- フレームデータ の読み込み
- フレーム単位での全ノード変換計算

**public メソッド**:

```cpp
class BvhFile {
public:
    // ファイル読み込み
    bool load(const QString& filePath);
    bool isValid() const;
    
    // フレーム操作
    void setCurrentFrame(int frameIndex);
    int getFrameCount() const { return motion.frameCount; }
    double getFrameTime() const { return motion.frameTime; }
    double getTotalDuration() const { return motion.frameCount * motion.frameTime; }
    
    // ノード検索
    BvhNode* getRootNode() const { return rootNode; }
    BvhNode* getNodeByName(const QString& name) const;
    QList<BvhNode*> getAllNodes() const { return flatNodeList; }
    
    // 情報
    int getTotalChannels() const { return totalChannels; }
};
```

---

## パース処理の流れ

### ステップ 1: HIERARCHY ブロック解析

1. ファイルを行単位で読み込み
2. "HIERARCHY" キーワードを検索
3. 再帰的に ROOT ノードを解析
   - "ROOT" キーワード → ノード作成
   - "OFFSET x y z" → offset 設定
   - "CHANNELS n name1 name2 ..." → チャンネルリスト作成
   - "{" で子ノード群を再帰的に解析
   - "JOINT" キーワード → 子ノード作成（同じ処理）
   - "End Site" → hasEndSite = true, endSiteOffset 設定
   - "}" でノード終了
4. ノードツリーを構築

### ステップ 2: チャンネルレイアウト構築

1. ノードツリーを DFS 順で走査
2. 各ノードに channelStartIndex を割り当て
   - 最初のノード = 0
   - 次のノード = 前のノード + 前のノードのチャンネル数
   - ...
3. flatNodeList に DFS 順でノードを格納
4. totalChannels = 最後の channelStartIndex + 最後のノードのチャンネル数

### ステップ 3: MOTION ブロック解析

1. "MOTION" キーワード検索
2. "Frames:" で frameCount 読み込み
3. "Frame Time:" で frameTime 読み込み
4. 各フレーム行を読み込み
   - トークン化（空白・改行で分割）
   - double 値に変換
   - frames 配列に格納

### ステップ 4: フレーム適用

BvhFile::setCurrentFrame(int frameIndex) 内で：

1. frames[frameIndex] から各ノードのチャンネル値を復元
2. 各ノードの updateLocalTransform() を呼び出し（ローカル変換計算）
3. ノードツリーを DFS 順で走査
   - worldTransform = parent.worldTransform × node.localTransform
4. 結果をキャッシュ

---

## 変換計算の詳細

### ローカル変換計算（BvhNode::updateLocalTransform()）

1. **位置を抽出**: チャンネル内から Xposition, Yposition, Zposition の値を検索
   - あればそれを使用（ルートノード等）
   - なければ offset のみ使用

2. **回転を抽出**: チャンネル内から Xrotation, Yrotation, Zrotation の値を検索
   - チャンネル列の順序に従い、その順で回転行列を合成

3. **行列合成**:
   ```
   localTransform = Translation(offset + position) × Rotation
   ```
   ※ Rotation は、チャンネルで指定された順序で合成（例: Z→X→Y なら、その順）

### ワールド座標系変換（BvhFile::setCurrentFrame()）

DFS 順で各ノードを訪問：

```
for each node in DFS order {
    if (node is root) {
        node.worldTransform = node.localTransform;
    } else {
        node.worldTransform = parent.worldTransform × node.localTransform;
    }
    for each child in node.children {
        visitDFS(child);  // 再帰
    }
}
```

---

## メモリ効率と性能

### フレーム毎計算戦略

現在のアプローチ：各 setCurrentFrame() 呼び出し時に、全ノードのワールド座標系を毎回計算

**利点**:
- シンプル（デバッグ容易）
- キャッシュ無効化の複雑性がない

**将来最適化**:
- dirty flag を追加して変更ノードのみ再計算
- GPU コンピュートシェーダで並列計算
- アニメーション補間（フレーム間の中間値）

### メモリ使用量

- ノード 1個あたり：Channel × n + QMatrix4x4 × 2 + QVector3D × 2 ≈ 500-1000 bytes
- フレームデータ 1個あたり：double × channelCount ≈ 8 × 100-200 = 800-1600 bytes
- 例）100 ノード + 3000 フレーム = 50KB + 2.4MB ≈ 2.5MB（実用的）

---

## ヘッダーファイルの構成

```
src/core/bvhfile/
  ├── channel.h         // Channel 列挙・クラス
  ├── channel.cpp
  ├── bvhnode.h         // BvhNode クラス
  ├── bvhnode.cpp
  ├── bvhmotion.h       // BvhMotion クラス
  ├── bvhmotion.cpp
  ├── bvhfile.h         // BvhFile クラス（パブリックインターフェース）
  └── bvhfile.cpp
```

---

## Qt Quick 統合（将来）

BvhFile を QObject 継承にすることで、QML から直接呼び出し可能にする：

```cpp
class BvhFile : public QObject {
    Q_OBJECT
    Q_PROPERTY(int frameCount READ getFrameCount NOTIFY frameCountChanged)
    // ...
public slots:
    void setCurrentFrame(int frameIndex);
signals:
    void frameCountChanged();
    // ...
};
```

Qt 3D と組み合わせて、BvhNode の worldTransform から Model Entity の位置・回転を更新。

---

## 参考

- [BVH ファイル仕様](../specs/bvh-file.md)
- サンプルファイル: `bvhfiles/` 配下
