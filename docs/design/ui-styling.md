# UI スタイリング方針

## 目的

BVH Viewer の 2D UI（メニュー、サイドバー、アニメーションコントロール等）の見た目を一貫させ、OS のライト／ダーク設定への追従と保守性を確保する。

3D ビュー（Qt Quick 3D）内の描画色は本ドキュメントの Palette 方針の対象外とする。

---

## Qt Quick Controls スタイル

**Fusion スタイル**を採用する。

* デスクトップ向けのプラットフォーム非依存スタイルである。
* システム設定に応じてライト／ダークテーマを自動切り替えする。
* Qt Widgets の Fusion スタイルと同系統のデザイン言語を持つ。

参考: [Fusion Style | Qt Quick Controls](https://doc.qt.io/qt-6/qtquickcontrols-fusion.html)

### 適用方法

C++ 側（`main.cpp` 等）でアプリケーション起動前に Fusion スタイルを指定する。

```cpp
#include <QQuickStyle>

// QGuiApplication 生成前
QQuickStyle::setStyle("Fusion");
```

QML 側では `import QtQuick.Controls` により Fusion スタイルの各コントロール（`Button`, `Slider`, `Switch`, `MenuBar` 等）をそのまま使用する。個別コントロールに別スタイルを指定しない。

---

## 色の扱い

### 基本方針: Palette を使用する

2D UI で使用する色は、**Qt Quick の `Palette` が提供する色**を基本とする。16 進数リテラル（`"#121212"`）や名前付き色（`"white"`）の直接指定は避ける。

* ウィンドウやコントロールは `palette.window`, `palette.windowText`, `palette.base`, `palette.text`, `palette.button`, `palette.buttonText`, `palette.highlight`, `palette.highlightedText`, `palette.mid`, `palette.midlight`, `palette.dark`, `palette.light`, `palette.shadow` 等を参照する。
* 親アイテム（通常は `ApplicationWindow`）で `palette` プロパティを設定すると、子要素へ自動的に伝播する。
* Fusion スタイルはシステムの `SystemPalette` を基に色を決定するため、OS テーマとの整合性が保たれる。

参考: [Palette QML Type](https://doc.qt.io/qt-6/qml-qtquick-palette.html)

#### 記述例

```qml
ApplicationWindow {
    // 必要に応じてアプリ全体のパレットを上書き（任意）
    // palette.highlight: "#569CD6"

    color: palette.window

    Rectangle {
        color: palette.base
        border.color: palette.mid

        Text {
            color: palette.text
            text: qsTr("Label")
        }
    }
}
```

無効状態の色は `palette.disabled.*` を使用する。

```qml
Text {
    enabled: false
    color: palette.disabled.text
}
```

### 直接色指定を許容する例外

次の場合は、Palette より**意味のある直接色指定**を優先する。

| 用途 | 例 | 理由 |
| :--- | :--- | :--- |
| **意味付きの状態色** | エラー・警告・成功の表示 | Palette だけでは意味が伝わらない |
| **3D ビュー内** | スケルトン色、グリッド、背景、ライティング | Qt Quick 3D のマテリアル／シーン描画であり、Controls の Palette 対象外 |
| **スケルトン識別色** | 複数 BVH の色分け | データ識別のための意図的な固定色（要件 F03） |

#### 意味付きの状態色（推奨）

アプリ内で共通の定数として QML または C++ に定義し、散在するリテラルを避ける。

| 意味 | 用途例 |
| :--- | :--- |
| エラー（赤系） | ステータスバーの読み込み失敗メッセージ |
| 警告（黄／橙系） | 非致命的な注意喚起 |
| 成功（緑系） | 操作完了のフィードバック（将来） |

具体的な色値は実装時に決定する。OS テーマに依存させず、意味が常に読み取れることを優先する。

#### 3D ビュー内

`Bvh3DView.qml` および Qt Quick 3D の `Scene`, `Model`, `DefaultMaterial` 等では、シーン可読性のため Palette ではなく直接色指定を用いる。

* ビューポート背景
* 床面グリッド
* ジョイント／ボーンのマテリアル
* 複数スケルトンの識別色

---

## コントロールのカスタマイズ

Fusion スタイルのコントロールは、**デフォルトの見た目を尊重**する。

* `Button` 等に `background: Rectangle { color: "..." }` を付けて Palette を迂回するカスタム背景は、特別な理由がない限り行わない。
* 角丸・余白・フォントサイズなど、スタイルが提供する範囲で足りる場合は追加スタイルを付けない。
* レイアウト（`RowLayout`, `ColumnLayout`, `anchors`, `spacing`）による配置調整は問題ない。

---

## 関連ドキュメント

* [要件定義書 — UI 仕様](../requirements/requirements.md#44-ui仕様)
* [Architecture](./architecture.md)
