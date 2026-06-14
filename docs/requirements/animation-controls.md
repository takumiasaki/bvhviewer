# アニメーションコントロール要件・仕様書

## 1. 改訂履歴

| バージョン | 日付 | 改訂内容 |
| :--- | :--- | :--- |
| v1.0.0 | 2026/06/14 | 初版作成 |
| v1.0.1 | 2026/06/14 | 表示対象スケルトン限定・Frame Time 最小値基準・UI レイアウト固定の追記 |
| v1.0.2 | 2026/06/14 | duration を BVH 慣習 `(frameCount-1)×frameTime` に統一。タイムライン集計とポーズ駆動の対象を分離 |

---

## 2. 目的とスコープ

本書は、BVH Viewer における **シーン全体のアニメーション再生 UI**（再生・一時停止・シーク・時間表示）の要件と振る舞いを定義する。

### 2.1. 対象

- 3D ビュー下部のアニメーションコントロール（`Main.qml` の `animationController`）
- `SceneManager` が公開する再生関連プロパティとその更新ロジック
- QML 側のタイムライン駆動（`NumberAnimation` 等）

### 2.2. 対象外

- 各スケルトン（BVH オブジェクト）個別のタイムライン UI
- スケルトンごとの非同期再生（将来拡張 F10 / F11 の個別制御）
- 再生速度変更（スロー／倍速）
- ループ／ピンポン再生モードの選択 UI

### 2.3. 前提

- 複数 BVH を読み込んだ場合、**1 本のシーンタイムライン**でスケルトンを同期再生する（要件 F11）。
- 各 BVH の `Frame Time` に基づき `setPoseAtTime(t)` で補間再生する（要件 F11-2）。
- **タイムライン長さの算出**は読み込み済み・有効な **全スケルトン** を対象とする（表示状態・選択状態に依存しない）。一般的な DCC／ビューアと同様、非表示でもタイムライン上の存在は維持する。
- **ポーズ更新**（`setPoseAtTime`）は **表示中（visible）のスケルトンのみ** に適用する（描画対象外の計算を省略）。
- フレーム数・`Frame Time` が異なるスケルトンが混在しても、**シーンタイムライン上の同一時刻**を各表示中スケルトンに適用する。各スケルトンは自身のフレーム範囲内でクランプする（末尾以降は最終フレームを保持）。

---

## 3. 用語

| 用語 | 定義 |
| :--- | :--- |
| **タイムライン集計対象** | 読み込み済み・有効・`frameCount > 0` の全スケルトン。`duration` / `frameTime` / `frameCount` の算出に用いる。 |
| **ポーズ駆動対象** | `visible == true` のスケルトン。`setPoseAtTime` の呼び出し先。 |
| **シーンタイムライン** | タイムライン集計対象に基づく再生時間軸（秒）。UI と `SceneManager` が管理する唯一のタイムライン。 |
| **animationTime** | シーンタイムライン上の現在位置（秒）。内部の正とする再生位置。 |
| **currentFrame** | `animationTime` をシーン基準 `frameTime` で換算した表示用フレーム番号（整数）。シーク操作の補助表現。 |
| **duration** | シーンタイムラインの総再生時間（秒）。各スケルトンの `(frameCount - 1) × frameTime` の最大値。 |
| **frameCount（シーン）** | シーンタイムラインをフレーム単位で表したときの疑似的な総フレーム数。 |
| **frameTime（シーン）** | シーンタイムラインの基準フレーム間隔（秒）。タイムライン集計対象の `Frame Time` の **最小値**。 |
| **playing** | シーンタイムラインが時間方向に進行中かどうか。 |

---

## 4. シーンタイムラインの定義

### 4.0. 計算対象スケルトンの範囲（二層モデル）

タイムライン関連の処理は **集計** と **駆動** で対象が異なる。これにより、再生中の表示切替でタイムライン長が変動する問題を避けつつ、非表示スケルトンへの無駄な計算を省略する。

```
タイムライン集計対象 = { s | s.isValid() かつ s.frameCount > 0 }   // 読み込み済み全件
ポーズ駆動対象     = { s | s.visible == true } ∩ タイムライン集計対象
```

| 条件 | タイムライン集計 | ポーズ更新（`setPoseAtTime`） |
| :--- | :--- | :--- |
| 表示中 | 対象に含める | 更新する |
| 非表示 | **対象に含める**（長さ・基準レートの算出に使用） | **更新しない**（非表示直前のポーズを保持） |
| 選択中かどうか | **影響しない** | 影響しない |
| 読み込み済み 0 体 | §4.6 を適用 | — |

**設計根拠（一般的 UI との整合）:** Blender・Maya・Unity 等の 3D ツールでは、オブジェクトの表示／非表示（アイコン）は **描画のオン／オフ** が主目的であり、シーンタイムラインの長さや再生位置は通常 **シーン全体** で固定される。非表示にしてもクリップやキーフレームはタイムライン上に残る。本アプリもこれに倣い、タイムライン集計は全スケルトン、ポーズ評価のみ visible に限定する。

- スケルトンの追加・削除時に §4.1〜4.3 を再計算する。
- 表示／非表示の切り替え時は **タイムライン集計は再計算しない**（`duration` / `frameCount` / `frameTime` は不変）。
- 非表示→表示時は **現在の `animationTime` へ即同期** する（§13.3）。

### 4.1. 基準フレームレート（frameTime）

タイムライン集計対象スケルトンの `Frame Time` の **最小値** をシーン基準とする。

```
frameTime = min( s.frameTime | s ∈ タイムライン集計対象 )
```

- 集計対象が 0 体のとき `frameTime = 0`。
- 同一値が複数ある場合はその値を用いる（どれを選んでも同じ）。

### 4.2. 総再生時間（duration）

各タイムライン集計対象スケルトンの最終キーフレーム時刻を求め、その **最大値** をシーンの総再生時間とする。BVH 慣習に従い、最終フレーム（インデックス `frameCount - 1`）の時刻を用いる。

```
skeletonDuration(s) = (s.frameCount > 1) ? (s.frameCount - 1) × s.frameTime : 0
duration            = max( skeletonDuration(s) | s ∈ タイムライン集計対象 )
```

- 集計対象が 0 体のとき `duration = 0`。
- 1 フレームのみ（`frameCount == 1`）のスケルトンは `skeletonDuration = 0`（静止ポーズ）。
- UI のスライダー範囲および再生終了判定に使用する。

### 4.3. シーンの疑似的フレーム数（frameCount）

シーン全体をフレーム単位で表現するための **疑似的** な総フレーム数。

**推奨（本仕様の正）:**

```
frameCount = (frameTime > 0) ? floor( duration / frameTime ) + 1 : 0
```

- フレーム 0 が `animationTime = 0`、最終フレームが `animationTime = duration` に対応する。
- `frameTime` が全スケルトンで一致する場合、`frameCount = max(s.frameCount)` と一致する。
- フレーム表示・フレーム単位シークはこの値を用いる。

**代替（実装上の簡略化）:**

```
frameCount = max( s.frameCount | s ∈ タイムライン集計対象 )
```

- `duration / frameTime + 1` が整数にならない、または丸め誤差が問題になる場合に限り許容する。

### 4.4. currentFrame との関係

```
currentFrame = round( animationTime / frameTime )
currentFrame は [0, frameCount - 1] にクランプ
```

- `currentFrame` は **シーンタイムライン**上のフレーム番号であり、特定スケルトン・選択状態に依存しない。
- フレーム表示・フレーム単位シークのために導出する。正の再生位置は `animationTime` とする。

### 4.5. 各スケルトンへの適用

`animationTime` が更新されるたび、**ポーズ駆動対象**（表示中）スケルトンに `Bvh3DModel::setPoseAtTime(animationTime)` を呼び出す。

- 非表示スケルトンには呼び出さない（描画更新の対象外。タイムライン長さには引き続き寄与する）。
- スケルトンの `(frameCount - 1) × frameTime` がシーン `duration` より短い場合、そのスケルトンは **自身の最終フレームでクランプ** する。
- スケルトンの `Frame Time` がシーン基準と異なる場合も、**同一の animationTime（秒）** を渡す（各モデル内部で自身の `Frame Time` に従って補間する）。
- 非表示→表示時: `visibleChanged` で **即座に** `setPoseAtTime(animationTime)` を 1 回呼び、現在位置のポーズへ同期する。

### 4.6. 有効なモーションが 0 件のとき

読み込み済みスケルトンが 0 体、またはいずれも `frameCount == 0` の場合:

- `frameTime = 0`, `duration = 0`, `frameCount = 0`
- タイムライン UI は非表示または無効（AC-01-4）
- `playing = false` にする

**全非表示だが読み込み済みモーションあり** の場合は本節には該当しない。タイムライン UI は有効のまま、再生・シークが可能（表示中スケルトンが 0 体でもタイムラインは維持）。

---

## 5. 機能要件

### AC-01: 再生・一時停止（F08 具体化）

| ID | 要件 |
| :--- | :--- |
| AC-01-1 | Play 操作で `playing = true` とし、`animationTime` を時間方向に進行させる。 |
| AC-01-2 | Pause 操作で `playing = false` とし、進行を停止する。現在の `animationTime` を保持する。 |
| AC-01-3 | Play / Pause は 1 つのトグルボタンで切り替える。 |
| AC-01-4 | シーンに有効なモーションがない（`frameCount <= 0`）場合、コントロールを無効化または非表示とする。 |
| AC-01-5 | 1 フレームのみ（`frameCount == 1`）の場合、Play は無効とする（再生する意味がない）。 |

### AC-02: 再生終了時の振る舞い

| ID | 要件 |
| :--- | :--- |
| AC-02-1 | `animationTime` が `duration` に到達したら再生を **自動停止** する（`playing = false`）。 |
| AC-02-2 | 停止時、`animationTime` は `duration` に固定する（最終フレームを表示）。 |
| AC-02-3 | 停止後、Play ボタンは **「Play」** と表示する（Pause 表示のままにしない）。 |
| AC-02-4 | 終了状態から Play を 1 回押すと、**先頭（`animationTime = 0`）から再生** を開始する。 |
| AC-02-5 | 終了状態で Pause を押す必要はない（2 回クリック問題を解消する）。 |

**受け入れ基準（AC-02）:** 最終フレーム到達 → ボタンが Play 表示 → 1 回の Play クリックで先頭から再生開始。

### AC-03: シーク（タイムライン操作）（F09 具体化）

| ID | 要件 |
| :--- | :--- |
| AC-03-1 | スライダーは **シーンタイムライン**（0 〜 `duration` 秒、または 0 〜 `frameCount - 1` フレーム）を操作する。 |
| AC-03-2 | スライダー操作（ドラッグ中・離した後）により `animationTime` を更新し、**表示対象スケルトン** のポーズを即時反映する。 |
| AC-03-3 | **停止中**にシークした後 Play を押すと、**シークした位置から** 再生を再開する（先頭に戻さない）。 |
| AC-03-4 | **再生中**にシークした場合、**シークした位置で再生を継続** する（直前の再生位置に戻らない）。 |
| AC-03-5 | 再生中のシーク中（ドラッグ中）は、ユーザー操作を優先し、タイムライン表示は操作位置に追従する。 |

**受け入れ基準（AC-03）:**

- 停止 → 中間へシーク → Play → 中間から再生開始。
- 再生中 → スライダー移動 → 移動先で再生継続（スナップバックなし）。

### AC-04: 先頭へ戻す（Reset）

| ID | 要件 |
| :--- | :--- |
| AC-04-1 | Reset 操作で `animationTime = 0`、`playing = false` とする。 |
| AC-04-2 | Reset は **再生中は無効** とする（誤操作防止）。 |
| AC-04-3 | Reset 後、全スケルトンは先頭フレームのポーズを表示する。 |

### AC-05: 時間／フレーム表示

| ID | 要件 |
| :--- | :--- |
| AC-05-1 | タイムライン横のラベルは、**デフォルトで時間（秒）** を表示する。 |
| AC-05-2 | 表示形式例: `0.00 / 12.34 s`（現在時刻 / 総時間、小数第 2 位）。 |
| AC-05-3 | ユーザーが **フレーム表示に切り替え** できる（トグルまたはコンボ等）。 |
| AC-05-4 | フレーム表示形式例: `Frame 42 / 371`（`currentFrame` / `frameCount - 1`）。 |
| AC-05-5 | 表示モードの切り替えはシーンタイムライン基準の値を用い、**選択状態・特定スケルトンに依存しない**。 |
| AC-05-6 | スライダーの内部値は `animationTime`（秒）を正とし、フレーム表示時もシーン基準で換算する。 |

### AC-06: 選択状態・表示状態との関係

| ID | 要件 |
| :--- | :--- |
| AC-06-1 | サイドバーでスケルトンを選択（`activeIndex`）しても、タイムラインの `frameCount` / `duration` / `frameTime` / 表示ラベルは **変わらない**。 |
| AC-06-2 | `activeIndex` はオフセット編集・ステータス表示等にのみ使用し、再生タイムラインの算出には使わない。 |
| AC-06-3 | 読み込み済みスケルトンが 0 体のとき、タイムライン UI は非表示または無効。 |

### AC-07: 表示／非表示とタイムライン

| ID | 要件 |
| :--- | :--- |
| AC-07-1 | スケルトンの表示／非表示を切り替えても `frameTime` / `duration` / `frameCount` は **変わらない**（§4.0 二層モデル）。 |
| AC-07-2 | 非表示スケルトンは `setPoseAtTime` の対象外とし、ポーズ更新を行わない。 |
| AC-07-3 | 非表示→表示時、**現在の `animationTime` のポーズへ即同期** する（§4.5）。 |
| AC-07-4 | スケルトンの追加・削除時は §4.1〜4.3 を再計算する。再計算後 `animationTime > duration` ならクランプ（AC-02 連動）。 |
| AC-07-5 | 全非表示でもタイムライン UI は **有効** のまま（読み込み済みモーションが存在する限り）。 |

### AC-08: タイムライン UI のレイアウト安定性

| ID | 要件 |
| :--- | :--- |
| AC-08-1 | **再生中**、現在時刻・フレーム番号の桁数変化によってラベル幅が変わり、スライダーやボタンの位置が **ずれない** こと。 |
| AC-08-2 | 時間表示・フレーム表示とも、数値部分は **等幅フォント** または **固定最小幅**（`Layout.minimumWidth` 等）で桁揃えする。 |
| AC-08-3 | 固定幅は **現在の `duration` / `frameCount` に基づく最大桁数** で確保する（例: 総時間 123.45 s なら「000.00 / 123.45 s」分の幅）。 |
| AC-08-4 | 再生中に桁数が変わってもレイアウトが動かないことが優先。総時間が変わることによるレイアウト変更（スケルトン追加・削除・表示切替）は **許可** する（AC-08-5）。 |
| AC-08-5 | スケルトンの表示／非表示・追加・削除により `duration` や `frameCount` が変わり、ラベル幅やコントロール領域が再配置されることは許容する。 |

---

## 6. UI 仕様

### 6.1. レイアウト

3D ビュー下部の `animationController` に以下を左から配置する（要件 F12）。

```
[ Play/Pause ] [ Reset ] [ ─────── スライダー ─────── ] [ 時間/フレーム表示 ] [ 表示切替（任意）]
```

### 6.2. コントロール状態

| 条件 | Play/Pause | Reset | スライダー | 表示ラベル |
| :--- | :--- | :--- | :--- | :--- |
| 読み込み 0 体 | 非表示または無効 | 非表示または無効 | 非表示または無効 | 非表示 |
| 有効モーション 0 件 | 非表示または無効 | 非表示または無効 | 非表示または無効 | 非表示 |
| 全非表示（モーションあり） | 有効（Play/Pause） | 有効（停止中） | 有効 | 有効 |
| frameCount == 1 | 無効 | 有効 | 有効（実質固定） | 有効 |
| frameCount >= 2, 停止中 | 有効（Play） | 有効 | 有効 | 有効 |
| frameCount >= 2, 再生中 | 有効（Pause） | **無効** | 有効 | 有効 |
| 再生終了（末尾） | 有効（**Play**） | 有効 | 有効 | 有効 |

### 6.3. ボタンラベル

- `playing == true` → 「Pause」
- `playing == false` → 「Play」（末尾到達後も同様）

### 6.4. 時間／フレームラベルのレイアウト（AC-08）

```
推奨レイアウト例:

  Label {
      font.family: fixedFontFamily   // 等幅
      Layout.minimumWidth: computedLabelWidth
      horizontalAlignment: Text.AlignRight
      text: formatTime(current, total)   // 例 "  12.34 / 123.45 s"
  }
```

- `computedLabelWidth` は `duration` および `frameCount` 変更時（追加・削除・表示切替）に再計算する。
- **再生中の `animationTime` / `currentFrame` 変化だけでは `computedLabelWidth` を変えない**（ゼロパディング等で吸収）。
- フレーム表示例: `"Frame %1 / %2".arg(paddedCurrent).arg(paddedTotal)` — 総フレーム桁数に合わせて左ゼロ埋めまたはスペース埋め。

---

## 7. データモデル（SceneManager 公開 API）

### 7.1. プロパティ

| プロパティ | 型 | 説明 |
| :--- | :--- | :--- |
| `playing` | bool | 再生中か |
| `animationTime` | qreal | 現在位置（秒）。**正の再生位置** |
| `duration` | qreal | シーン総時間（秒）。読み取り専用 |
| `currentFrame` | int | 表示・フレームシーク用。`animationTime` と同期 |
| `frameCount` | int | **シーン**のフレーム数。読み取り専用 |
| `frameTime` | double | **シーン**基準のフレーム間隔。読み取り専用 |

### 7.2. メソッド

| メソッド | 動作 |
| :--- | :--- |
| `play()` | `playing = true`。`animationTime >= duration` のときは先に `animationTime = 0` してから開始（AC-02-4） |
| `pause()` | `playing = false` |
| `toggle()` | `playing` を反転 |
| `setAnimationTime(t)` | `t` を `[0, duration]` にクランプし **表示対象スケルトン** に適用。`currentFrame` を同期 |
| `setCurrentFrame(n)` | `animationTime = n * frameTime` を経由して更新（シーン基準） |

表示対象の変化を検知するため、`BvhSkeletonItem::visibleChanged` 時は **ポーズ同期のみ**（`setPoseAtTime` 1 回）。スケルトン追加・削除時に `recalculateTimeline()`（仮称）を呼び出し、`frameTimeChanged` / `durationChanged` / `frameCountChanged` を emit する。

### 7.3. 更新の単一ソース

- **正:** `animationTime`
- `currentFrame` は `animationTime` から導出する派生値として扱う。
- スライダー・`NumberAnimation`・シーク操作はすべて `animationTime` を更新する。
- `setCurrentFrame` はフレーム単位 UI の便宜上の入口とし、内部では必ず `animationTime` に変換する。

### 7.4. activeIndex・visible との関係（修正方針）

| 項目 | 方針 |
| :--- | :--- |
| `activeIndex` | タイムライン算出に **使用しない** |
| `visible` | タイムライン集計には **影響しない**。`setPoseAtTime` 対象の **フィルタ条件** |
| `frameCount()` / `frameTime()` / `duration` | 常に §4 の表示対象集約値を返す（`activeScene` 優先の現行実装は修正） |
| `syncCurrentFrameFromModels()` | アクティブスケルトンの `currentFrame` で上書きせず、`animationTime` から導出 |
| `applyAnimationTimeToAll()` | 表示対象スケルトンのみに `setPoseAtTime` を呼ぶ |

---

## 8. QML 再生駆動仕様

### 8.1. 原則

`NumberAnimation` をそのまま `running: playing` にバインドし、`from: 0` 固定で使う方式は **採用しない**（以下の問題があるため）。

- 再生開始のたびに `animationTime` が 0 にリセットされる
- 固定 `to` / `duration` が実際の `duration` と一致しない
- シーク後にアニメーション内部時刻と `animationTime` が乖離しスナップバックが起きる

### 8.2. 推奨方式: フレームタイマー駆動

`FrameAnimation` または `Timer`（`interval: 16` ms 程度）で経過時間を積算する。

```
onTriggered / onFrame:
  if (!playing) return
  animationTime = min(animationTime + deltaSeconds, duration)
  if (animationTime >= duration)
    playing = false   // AC-02
```

- `deltaSeconds` は実経過時間（`deltaTime`）を用い、Wall clock ベースとする。
- シーク時は `animationTime` を直接設定するだけで、タイマー側の内部オフセットは持たない（単一ソース）。

### 8.3. シークと再生の連携

| 操作 | QML / C++ の処理 |
| :--- | :--- |
| スライダー `onMoved` / `onPressedChanged` | `sceneManager.animationTime = sliderValue`（秒） |
| Play（途中位置） | 既存 `animationTime` を維持したまま `playing = true` |
| Play（末尾から） | `play()` 内で `animationTime = 0` 後 `playing = true` |
| Pause | `playing = false` のみ（`animationTime` は変更しない） |

### 8.4. スライダーバインディング

- `from: 0`, `to: sceneManager.duration`
- `value: sceneManager.animationTime`
- ユーザー操作中（`pressed` / `moved`）は双方向バインディングの競合を避けるため、必要に応じ `pressed` 中は `value` のモデル→UI 更新を抑制する。

---

## 9. 既知課題と本仕様での解消方針

| # | 現象 | 原因（現状） | 本仕様での方針 |
| :--- | :--- | :--- | :--- |
| 1 | 選択オブジェクトのフレーム数が UI に反映される | `frameCount()` が `activeScene` 優先 | AC-06, §4.2, §7.4 |
| 2 | フレーム表示のみで時間感が得にくい | UI が `currentFrame` のみ表示 | AC-05（デフォルト時間表示） |
| 3 | 再生完了後も Pause 表示、2 回クリック必要 | `NumberAnimation` が `running` のまま／終了時 `playing` 未解除 | AC-02, §8.2 |
| 4 | 再生中シークがスナップバック | `NumberAnimation` 内部時刻と `animationTime` の二重管理 | AC-03-4, §8.1–8.3 |
| 5 | 停止中シーク後 Play で先頭から再生 | 再生開始時に `from: 0` へリセット | AC-03-3, §8.3 |

---

## 10. テスト観点（受け入れチェックリスト）

### 10.1. シーンタイムライン

- [ ] フレーム数 100 / 200、Frame Time 異なる 2 体を読み込み。選択を変えても `duration` / `frameCount` が変わらない
- [ ] `duration = max((frameCount - 1) × frameTime)`、`frameTime = min(frameTime)` であること
- [ ] `frameCount = floor(duration / frameTime) + 1`（推奨式）または代替式のいずれかで一貫していること
- [ ] 短い方のスケルトンはシーン末尾付近で最終ポーズを保持
- [ ] 長い方を非表示にしても `duration` / `frameCount` は **変わらない**（AC-07-1）
- [ ] スケルトン削除で `duration` が短くなる場合のみタイムライン長が変わる

### 10.1b. 表示／非表示

- [ ] 非表示スケルトンは再生中もポーズが更新されない
- [ ] 再生中にスケルトンを非表示→表示しても `animationTime` / スライダー位置は変わらない
- [ ] 非表示→表示時、現在位置のポーズが即反映される（§13.3）
- [ ] 全非表示でもタイムライン操作・再生が可能（表示中が 0 体でも UI 有効）

### 10.2. 再生終了

- [ ] 最終フレーム到達後、ボタンが Play 表示になる
- [ ] その状態から Play 1 回で先頭から再生

### 10.3. シーク

- [ ] 停止中: 中間シーク → Play → 中間から再生
- [ ] 再生中: スライダードラッグ → 離した位置で継続（元位置に戻らない）

### 10.4. Reset

- [ ] 途中停止 → Reset → 先頭フレーム表示、`playing == false`
- [ ] 再生中 Reset は無効

### 10.5. 表示

- [ ] デフォルトで `秒 / 総秒` 表示
- [ ] フレーム表示切替でシーン基準フレーム番号が表示される
- [ ] 再生中、フレーム番号／秒の桁が変わってもスライダー・ボタン位置がずれない（AC-08）
- [ ] スケルトン追加・削除・表示切替で総時間桁が変わる場合はレイアウト変更が起きてよい（AC-08-5）

---

## 11. 関連ドキュメント

| ドキュメント | 関係 |
| :--- | :--- |
| [requirements.md](./requirements.md) | F08, F09, F11, F11-2, F12 の上位要件 |
| [scene-manager.md](../design/scene-manager.md) | SceneManager 設計（§「フレーム数の扱い」は本書に従い更新予定） |
| [architecture.md](../design/architecture.md) | アニメーション駆動の全体像 |

---

## 12. 将来拡張（本書スコープ外）

- ループ再生 ON/OFF
- 再生速度（0.25x, 0.5x, 2x 等）
- スケルトン個別の再生／停止
- タイムライン上のキーフレームマーカー
- スクラブ中の一時停止（ドラッグ開始で Pause、離して Play 再開）— 必要になったら別途仕様化

---

## 13. 実装上の課題・注意点

### 13.1. 疑似的 `frameCount` の端数

`frameTime` がスケルトン間で異なる場合、`floor(duration / frameTime) + 1` が `max(frameCount)` と一致しないことがある。許容できない場合は §4.3 の **代替式**（最大 `frameCount`）を用いる。

### 13.2. 非表示スケルトンのポーズ

非表示中は `setPoseAtTime` を呼ばないため、再表示時に **現在の `animationTime` へ一発で同期** する必要がある（`visibleChanged` ハンドラで 1 回呼び出し）。これを怠ると、非表示直前の古いポーズが一瞬表示される。

### 13.3. 全非表示時の再生

タイムライン長は維持されるため、表示中スケルトンが 0 体でも `playing == true` となりうる。ポーズ更新対象が 0 件でも `animationTime` は進行し、後から表示を戻すとその位置から再生されている。これは意図した振る舞い（タイムラインはシーン全体のもの）。

### 13.4. UI 固定幅の算出タイミング

- **再計算トリガー:** `durationChanged` / `frameCountChanged`（＝スケルトン追加・削除）
- **再計算しない:** `animationTimeChanged` / `currentFrameChanged` / `visibleChanged`

等幅フォントが利用できない環境では、`TextMetrics` で最大文字列幅を事前計算する。
