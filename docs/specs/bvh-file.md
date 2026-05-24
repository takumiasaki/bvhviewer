% BVH ファイル仕様

概要
----
BVH (Biovision Hierarchy) はモーションキャプチャデータを格納するシンプルなテキストフォーマットです。
ファイルは大きく2つのブロックで構成されます。

- HIERARCHY: 階層（スケルトン）の構造定義
- MOTION: フレーム単位のモーションデータ

本仕様書は `bvhfiles/` にあるサンプル（例: aachan.bvh, kashiyuka.bvh, nocchi.bvh）を参照して一般的なBVHの振る舞いを整理したものです。

基本構造
----

1) HIERARCHY ブロック

キーワードとブロック構成の例:

HIERARCHY
ROOT <name>
{
    OFFSET x y z
    CHANNELS <n> <channel1> <channel2> ...
    JOINT <name>
    {
        ...
    }
    End Site
    {
        OFFSET x y z
    }
}

説明:
- `ROOT`, `JOINT` はノード（ボーン）を表します。`End Site` はリーフ（末端）位置を表現します。
- `OFFSET x y z` は親ノードからの相対オフセット（通常は同じ単位系）です。BVH自体は単位を明示しないため、メートル・センチ・ミリ等はデータの由来に依存します。
- `CHANNELS` はそのジョイントで与えられるチャンネル数とチャンネルの種類を列挙します。一般的なチャンネル名は `Xposition` `Yposition` `Zposition` `Xrotation` `Yrotation` `Zrotation` です。

チャンネル順序
----

BVHではチャンネルの並び順が非常に重要です。ルートノードは位置（position）と回転（rotation）を持つことが多く、子ノードは回転のみを持つのが一般的です。

例:
`CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation`

上の例ではフレームデータ上のこのジョイントに対応する値は6個で、まず3つが平行移動（rootのワールド空間における平行移動）に対応し、後ろの3つが回転（通常は度単位）に対応します。

回転の扱い
----

- 回転チャンネルは度（degrees）で記録されるのが標準的です。
- 回転は各チャンネルで指定された順序で適用されます（チャンネル列の順序が回転の合成順序になる）。
- 多くのBVHはチャンネルに `Xrotation Yrotation Zrotation` の順で指定され、この順序で回転を適用する実装が多いです。実装時はチャンネル順をファイルから厳密に読み取り、その順序で回転行列（またはクォータニオン）を合成してください。

End Site
----

`End Site` ブロックは子を持たない終端の位置を示します。内部には `OFFSET` のみを持ちます。レンダリングでは末端の位置に小さな球やマーカーを描画することが多いです。

2) MOTION ブロック
----

例:
MOTION
Frames: 100
Frame Time: 0.0333333
<frame0_values>
<frame1_values>
...

- `Frames:` はフレーム総数を示します。
- `Frame Time:` は各フレームの時間（秒）を示します（例: 0.0333333 は30 FPSに相当）。
- 各フレーム行はHIERARCHYで定義された全チャンネルを順番に並べた数値列です。

フレームデータの並び方
----

全フレームの各行は、HIERARCHYでトップダウンに定義されたジョイント順（通常は宣言順）に従い、各ジョイントごとに `CHANNELS` で指定されたチャンネル分の値を持ちます。
したがって、パーサはまず階層を順に走査して「チャンネルレイアウト」を構築し、それから各フレーム行をトークン化して対応するチャンネルへ値を割り当てます。

パース手順の概略
----

1. HIERARCHY を読み、ノードツリーを構築する。
2. 各ノードについて `OFFSET` と `CHANNELS`（チャンネル数と名前）を記録する。チャネルが持つ順序を保持する。
3. `MOTION` ブロックで `Frames` と `Frame Time` を読み取る。
4. 各フレーム行の数値を、手順2で構築したチャンネルリストの順に割り当てる。
5. 各ジョイントのローカル変換（平行移動と回転）を復元し、必要に応じてジョイントのワールド変換を順に合成する。

注意点 / 落とし穴
----

- 回転の適用順序（チャンネル列）が実装に影響を与える。ファイルが指定する順序で行うこと。
- 回転は度で与えられることを前提にする実装が多いが、ファイルによっては異なる場合があるため注意。
- ルート以外のジョイントには位置チャンネルがない場合が多い（その場合は `OFFSET` のみを使う）。
- フレーム行のトークン数が期待数と一致しない場合はファイル破損の可能性あり。

簡単なBVH断片例
----

HIERARCHY
ROOT Hips
{
  OFFSET 0.00 0.00 0.00
  CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
  JOINT Chest
  {
    OFFSET 0.00 10.00 0.00
    CHANNELS 3 Zrotation Xrotation Yrotation
    End Site
    {
      OFFSET 0.00 5.00 0.00
    }
  }
}
MOTION
Frames: 2
Frame Time: 0.0333333
0.00 0.00 0.00 0.0 0.0 0.0  0.0 0.0 0.0
0.00 0.10 0.00 1.0 0.5 0.0  0.1 0.0 0.0

付録: 実装チェックリスト
----

- HIERARCHY の正しいネスト解析
- `OFFSET` の読み取りと格納
- `CHANNELS` の順序保持
- `MOTION` のフレーム数・フレームタイム読み取り
- 各フレーム行をチャンネル列にマッピング
- 回転合成の順序（ファイル指定どおり）でワールド変換を生成

参考
----
- サンプルBVH: bvhfiles/ 配下の .bvh ファイルを参照してください。

-- end of document
