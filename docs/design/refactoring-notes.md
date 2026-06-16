# 将来リファクタリングメモ

優先度は低い。現状の挙動に問題はなく、余裕ができたときに検討するためのメモ。

---

## 1. ~~`SceneManager.playing` を QML 側へ移す~~（実施済み）

### 実施内容（2026-06）

- `playing` 状態と再生ループ（`Timer`）を `components/TimelineBar.qml` に集約。
- `play()` / `pause()` / `toggle()` のロジックも QML 側へ移行（末尾到達時の先頭リセット、再生不可条件のチェック含む）。
- C++ `SceneManager` から `playing` プロパティと `play()` / `pause()` / `toggle()` を削除。
- シーン全削除時（`frameCount == 0`）は `TimelineBar` が `playing = false` にする。

### 残すもの

- `animationTime` / `currentFrame` / `duration` は SceneManager に残す（`setAnimationTime` が C++ で `setPoseAtTime` を呼ぶため）。

### 関連

- [scene-manager.md](./scene-manager.md) — 設計ドキュメントは「再生状態を SceneManager 管理」の記載が残っている。§3 参照。
- [animation-controls.md](../requirements/animation-controls.md) — 要件上の `playing` は QML（TimelineBar）が保持。

---

## 2. スケルトン色プロパティの整理

### 現状

色関連は 3 層にまたがっている。

| 層 | 色関連 |
|----|--------|
| `Bvh3DModel` | `jointColor`, `boneTone`, `customBoneColor`, `boneColorMode`, effective な `boneColor`（キャッシュ） |
| `BvhSkeletonItem` | 上記をほぼそのまま委譲（シグナル転送のみ） |
| `SceneManager` | パレット (`colorForIndex`)、読込時・Reset 時の初期化 (`resetSkeletonColors`)、`paletteIndex` |

仕様自体は [skeleton-colors.md](../requirements/skeleton-colors.md) v2.2.0 で整理済み。問題は **責務の置き場所** と **重複**。

### 論点

1. **3D Data Model に色を置く必然性**
   - 色は Pose 計算と無関係な UI メタデータ。`3d-data-model.md` も「Scene Manager 経由で QML に渡すメタデータ」と記載している。
   - 将来、`Bvh3DModel` をテストや別 UI から単体利用する場合、色プロパティがノイズになる。

2. **`BvhSkeletonItem` のパススルー**
   - 5 プロパティ × getter/setter × シグナル接続がほぼボイラープレート。
   - 色の所有を `BvhSkeletonItem`（または SceneManager エントリ）に寄せ、`Bvh3DModel` から外す案がある。

3. **`boneColor` のキャッシュ**
   - `effectiveBoneColor()` で計算可能な読取専用値を `m_boneColor` に保持し、`applyEffectiveBoneColor()` で同期している。
   - READ で都度計算するか、`QProperty` / bindable で derived にすると setter 連鎖が単純化できる可能性がある（要ベンチマークは不要な規模）。

4. **`colorFromTone` の所在**
   - 静的メソッドとして `Bvh3DModel` にあるが、トーン計算は 3D モデル本体の責務ではない。
   - QML (`ToneColorBar.qml`) からも呼ばれている。小さな `SkeletonColorUtils`（C++ または QML singleton）へ切り出す案。
   - 旧 `SceneColorUtils.js` は削除済み。ユーティリティの置き場所を一本化したい。

5. **SceneManager の初期化ロジック重複**
   - `loadScene` と `resetSkeletonColors` が同じ 4 プロパティ設定列を持つ。`applyDefaultAppearance(BvhSkeletonItem*)` 等にまとめられる。

### 方針案（段階的）

| 段階 | 内容 | 影響 |
|------|------|------|
| A（小） | 初期化・Reset の重複を SceneManager 内関数に集約 | 低リスク |
| B（中） | `colorFromTone` / `defaultBoneTone` を util へ移動 | QML import 変更のみ |
| C（大） | 色状態を `Bvh3DModel` から `BvhSkeletonItem`（または専用 struct）へ移動 | 層の境界が明確になる。テスト・設計ドキュメント更新が必要 |

段階 C を行う場合、`Bvh3DView.qml` は引き続き `skeleton.jointColor` / `skeleton.boneColor` を参照すれば QML 側の変更は限定的。

### 関連

- [3d-data-model.md](./3d-data-model.md) § スコープ「識別色」
- [scene-manager.md](./scene-manager.md) § 色分け
- [skeleton-colors.md](../requirements/skeleton-colors.md)

---

## 3. その他（メモ程度）

- **設計ドキュメントと実装の差分**: `scene-manager.md` は `NumberAnimation` 駆動・`SceneManager.playing` を記載しているが、実装は `TimelineBar.qml` の `Timer` + `animationTime` 積算、再生フラグは QML 側。リファクタ時にドキュメントを実装に合わせてよい。
- **`architecture.md`**: 「Scene Manager が再生状態を管理」とあるが、§1 実施後は「タイムライン（秒・フレーム）は C++、再生/停止フラグは QML（TimelineBar）」に書き換え候補。
