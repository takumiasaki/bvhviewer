# Qt Quick Integration Design

## 目的
Qt Quick で BVH ビューアの UI を構築するために、データモデルと表示層を分離する。最初は単一 BVH の骨描画を実装し、将来的な複数 BVH / シーン拡張に備える。

## 層構成
- `BvhFile` (`src/core/bvhfile.h/cpp`)
  - BVH のパースとフレーム適用を担当する純粋なデータ層
  - QML に依存しない
- `BvhSceneModel` (`src/ui/bvhscenemodel.h/cpp`)
  - `BvhFile` を所有し、QML からのロードと再生制御を提供する橋渡し層
  - `currentFrame`, `frameCount`, `frameTime`, `valid` を Q_PROPERTY として公開
- `BvhViewItem` (`src/ui/bvhviewitem.h/cpp`)
  - QML 上の描画コンポーネント
  - `BvhSceneModel` を参照し、現在の BVH の姿勢を 2D 風にレンダリングする

## 役割分担
- `BvhFile`:
  - 階層構造の構築
  - フレームデータの読み込み
  - 各ノードのローカル / ワールド変換計算
- `BvhSceneModel`:
  - ファイル読み込みの開始
  - フレーム切り替えの制御
  - QML に公開するプロパティとシグナルの管理
- `BvhViewItem`:
  - 表示のみ担当
  - `sceneModel.currentFrame` の変化を描画に反映

## QML 連携
- `main.cpp` で `BvhSceneModel` と `BvhViewItem` を QML に登録する
- `Main.qml` で `BvhSceneModel` と `BvhViewItem` を配置し、再生制御は QML 側で行う

## 今後の拡張
- 複数 BVH に対応するために `BvhScene` 層を追加し、複数 `BvhSceneModel` を同時管理する
- 2D プロトタイプから Qt Quick 3D への移行を段階的に検討する
- シーン管理、カメラ、ライト、メッシュ化などは別ドキュメントで整理する
