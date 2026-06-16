# スケルトン色選択・Active Scene パネル 要件・仕様書

## 1. 改訂履歴

| バージョン | 日付 | 改訂内容 |
| :--- | :--- | :--- |
| v1.0.0 | 2026/06/15 | 初版作成 |
| v1.0.1 | 2026/06/15 | Appearance ダイアログを ColorDialog + トーンスライダー方式に変更 |
| v1.0.2 | 2026/06/15 | Preview ボタン追加（のち廃止） |
| v2.0.0 | 2026/06/15 | ダイアログ廃止。Active Scene 3 タブ化。即時反映 + Reset to default 統一 |
| v2.0.1 | 2026/06/15 | スナップショット・暗黙復元を廃止。即時反映 + Reset のみ |
| v2.1.0 | 2026/06/16 | Appearance: `colorsLinked` を Bone モード ComboBox（Same / Tone / Custom）に変更。デフォルト Tone offset |
| v2.2.0 | 2026/06/16 | `boneTone` と `customBoneColor` を独立保持。`boneColor` はモードに応じた effective 色 |
| v2.3.0 | 2026/06/16 | 実装状況タグ追加（backlog 連携） |

**実装状況:** `[実装済]` — 残件・タグ凡例は [backlog.md](./backlog.md) を参照。

---

## 2. 目的

スケルトン（BVH 1 ファイル分）の **配置（Offset）** と **色（Appearance）** をサイドバーから編集する。  
複数 BVH 読込時は **スケルトンごとに異なる識別色** を自動付与する。

関連: [要件定義書](./requirements.md) F03, [UI スタイリング方針](../design/ui-styling.md)

---

## 3. 操作モデル（パッケージ A）

Active Scene パネル内の **Offset / Appearance は即時反映** とする。Preview / OK / Cancel / Revert ボタンは **置かない**。

| 操作 | 挙動 |
| :--- | :--- |
| **通常編集** | SpinBox / スライダー / CheckBox / ColorDialog 確定 → **即座に** `BvhSkeletonItem` へ反映 |
| **Reset to default** | 各タブの意味上の初期値へ **即座に** 反映 |
| **シーン切替** | `activeIndex` を切替。編集内容はモデルに **保持** |
| **Active 解除（×）** | `activeIndex = -1`。編集内容はモデルに **保持** |

---

## 4. 複数 BVH 読込時の色分け

| 項目 | 仕様 |
| :--- | :--- |
| **方針** | 読込順にパレット色を循環割当 |
| **初期状態** | ジョイント色 = パレット色。`boneTone = -25`、`customBoneColor = colorFromTone(joint, -25)`、`boneColorMode = ToneOffset` |
| **パレット** | `#67C1FF`, `#FF9F5E`, `#7BF59F`, `#D17BFF`, `#FFD464` |
| **paletteIndex** | 読込時 index を固定。Reset to default（Appearance）は `colorForIndex(paletteIndex)` |

---

## 4.1 ボーン色データモデル

| プロパティ | 役割 |
| :--- | :--- |
| `jointColor` | ジョイント色 |
| `boneTone` | Tone モード用パラメータ（-100〜+100）。**モード非依存で保持** |
| `customBoneColor` | Custom モード用パラメータ。**モード非依存で保持** |
| `boneColorMode` | SameAsJoint / ToneOffset / Custom |
| `boneColor` | **effective 色**（描画・スウォッチ用。読取専用） |

| モード | effective (`boneColor`) |
| :--- | :--- |
| SameAsJoint | `jointColor` |
| ToneOffset | `colorFromTone(jointColor, boneTone)` |
| Custom | `customBoneColor` |

**モード切替** … `boneTone` / `customBoneColor` は変更しない。`boneColorMode` 変更時に effective のみ再計算。

**joint 変更時** … Same / Tone では effective 再計算。Custom では `customBoneColor` 固定。

**Reset to default** … `jointColor`（パレット）、`boneTone = -25`、`customBoneColor = colorFromTone(joint, -25)`、`boneColorMode = ToneOffset`。

---

## 5. UI 構成

### 5.1 Scenes リスト

| 操作 | 挙動 |
| :--- | :--- |
| **行クリック**（チェックボックス・スウォッチ以外） | 当該シーンを Active にし、**Offset タブ** を表示 |
| **スウォッチクリック** | 当該シーンを Active にし、**Appearance タブ** を表示（行選択と同時） |
| **CheckBox** | 表示/非表示のみ（即時反映） |

### 5.2 Active Scene パネル（3 タブ）

```
Active Scene · {name}                         [×]
[ Offset ] [ Appearance ] [ Remove ]
─────────────────────────────────────────────
  （タブ内容）
```

#### Offset タブ

| 項目 | 仕様 |
| :--- | :--- |
| Offset X / Y | SpinBox。変更即反映 |
| Reset to default | `sceneOffset = (0, 0, 0)` を即反映 |

#### Appearance タブ

[ColorDialog](https://doc.qt.io/qt-6.8/ja/qml-qtquick-dialogs-colordialog.html) を使用。

| 項目 | 仕様 |
| :--- | :--- |
| Joint Choose color… | ColorDialog → `jointColor` 即反映。Same / Tone モード時は effective 再計算 |
| Bone モード ComboBox | Same / Tone offset（デフォルト）/ Custom。切替時に `boneTone` / `customBoneColor` は保持 |
| Tone カラーバー | `Tone offset` 時のみ **enabled**。`boneTone` を更新 |
| Bone Choose color… | `Custom color` 時のみ **enabled**。`customBoneColor` を更新 |
| Reset to default | `resetSkeletonColors(activeIndex)` 即反映 |

#### Remove タブ

| 項目 | 仕様 |
| :--- | :--- |
| 説明文 | 削除の意味を表示 |
| Remove scene… | 確認 `MessageDialog` の後 `removeScene` |

---

## 6. Active Scene の切替

- **別シーンへ切替** … `activeIndex` を更新。Offset / Appearance の編集は各 `BvhSkeletonItem` に保持される
- **× で解除** … `activeIndex = -1`。編集内容は保持される

---

## 7. DualColorSwatch（リスト用）

- Scenes 各行に 24×24 px、斜め 2 色（Joint 左上 / Bone 右下）
- クリック → Active 化 + Appearance タブ

---

## 8. 受け入れ条件

1. 複数 BVH で識別色が異なる
2. Offset / Appearance は編集即反映
3. Reset to default は各タブで即初期化
4. 行クリック → Offset タブ、スウォッチ → Appearance タブ
5. シーン切替・× で編集内容が保持される（直前状態への復元なし）
6. Remove は独立タブ + 確認ダイアログ

---

## 9. 関連ドキュメント

- [要件定義書](./requirements.md) — F03, F03-2
- [実装状況・残件リスト](./backlog.md)
- [Scene Manager 設計](../design/scene-manager.md)
