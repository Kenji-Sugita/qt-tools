# Trace Comparison Guide

## 目的

初回トレースと 2 回目以降のトレースを比較したい場合に、`qmlprofileranalyzer compare` と既存コマンドを使って誤誘導を減らしながら比較するための運用ガイドをまとめる。

`compare` は `hotspots` の `groupKey` ベース比較に限定する。詳細な原因確認は、比較後に各トレースの `hotspots` と `ranges` で行う。

## 基本方針

- 比較前に測定条件を揃える
- まず単一トレースとしてそれぞれを個別に読む
- `compare before.qzt after.qzt` で `hotspots` 差分を確認する
- `summary` ではなく `ranges` と `hotspots` を主に使う
- 差分の原因は断定せず、候補として扱う

## 比較前の条件

- 比較対象の操作を同一にする
- 端末、ビルド、設定、データ量を揃える
- 常時アニメーションなどのノイズ源は可能なら止める
- 初回と再訪の定義を明確にする
  - 初回: アプリ起動直後または画面初回表示
  - 再訪: 同じ操作を同じ条件で再度実行

## 推奨する採取方法

1. 初回トレースを採る
2. 同じ操作をもう一度行い、再訪トレースを採る
3. 必要なら再訪トレースを複数本採る

補足:

- 再訪は 1 本だけでなく複数本ある方がばらつきを見やすい
- 条件がずれると比較結果の意味が薄くなる

## 推奨する確認順

### 1. `compare`

`hotspots` の差分を先に見る。

```bash
./build/qmlprofileranalyzer compare before.qzt after.qzt
```

確認点:

- `status`
- `groupKey`
- `selfDelta`
- `selfDeltaPct`
- `callsDelta`
- `medianSelfDelta`
- `maxSelfDelta`

注意:

- `compare` は `hotspots` の比較に限定される
- `ranges` の親子関係差分は自動比較しない
- 1 回だけの差分はノイズの可能性がある

### 2. `summary`

全体傾向だけを見る。

確認点:

- `ranges`
- `pointEvents`
- `longestRange`
- `Compiling` の有無

注意:

- `totalDuration` は親子重複を含む
- `summary` だけで差分判断しない

### 3. `hotspots`

主要な比較対象。

確認点:

- `selfTotal`
- `groupKey`
- `calls`
- `medianSelf`
- `maxSelf`
- `outlier`
- `burst`
- `image`

見る観点:

- 初回だけ `Compiling` が大きいか
- 再訪で `selfTotal` が減っているか
- `calls` が増減していないか
- `outlier` や `burst` の出方が変わっていないか

### 4. `ranges`

差分の中身を掘るときに使う。

確認点:

- `selfTime`
- `focus`
- `gapWarning`

見る観点:

- 親子重複で見かけ上大きいだけではないか
- 真に重い leaf がどこか
- 初回だけ `gapWarning` が大きく出ていないか

## 差分の読み方

### 初回だけ `Compiling` が大きい

解釈:

- 大きい QML の初回処理の可能性がある
- ただし import や plugin load が混ざる可能性もある

### 初回だけ `image=yes` が付く hotspot が目立つ

解釈:

- 画像ロードや画像関連初期化の可能性がある
- ただし画像が唯一の原因とは断定しない

### 再訪でも `burst=yes` が残る

解釈:

- `ListView` や `Repeater` 相当の大量生成が継続している可能性がある
- ただし正常な更新処理の可能性もある

### `gapWarning=yes` が大きい

解釈:

- QML 内の子 range では説明しきれない処理がある可能性がある
- JavaScript, C++, import, I/O など複数候補を考える

## 比較時の注意

- `eventIndex` はトレース間で安定しない可能性がある
- 比較は `hotspots` の `groupKey` を優先し、必要に応じて `display`, `file`, `line`, `type` を見ながら行う
- 1 回だけの差分はノイズの可能性がある
- 平均値より `median` と `max` を優先して見る

## 実用上の結論

`compare` で差分の大きい `groupKey` を見つけ、単一トレース用の `hotspots` と `ranges` で原因候補を確認する。
比較は可能だが、測定条件がずれると結果の意味が薄くなるため、ツールの差分行だけで原因を断定しない。
