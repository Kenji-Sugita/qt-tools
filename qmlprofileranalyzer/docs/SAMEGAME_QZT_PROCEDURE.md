# QZT Evaluation Procedure

## 目的

この文書は、`.qzt` とそのトレース対象に対応するソースコードがあるときに、
`docs/SAMEGAME_QZT_EVALUATION.md` のような分析結果を作るための標準手順をまとめたもの。

この手順で再現したいもの:

- `.qzt` の全体統計
- Qt Creator と `qmlprofileranalyzer` の比較
- ソースコードと対応付けた重い処理と軽い処理の見立て

この文書は一般手順であり、
`samegame.qzt` はその具体例として扱う。

## 前提条件

### 作業ディレクトリー

以後のコマンドは次のディレクトリーで実行する。

```bash
cd /Users/sugita/src/qt/tools/qmlprofileranalyzer
```

### 必要な入力

最低限必要なものは次の 2 つ。

1. `.qzt` ファイル
2. その `.qzt` が記録した対象に対応するソースコード

一般条件:

- `.qzt` の内容とソースコードの系統が合っていること
- 行番号やファイル名が大きくずれていないこと

具体例:

- [samegame.qzt](/Users/sugita/src/qt/tools/qmlprofileranalyzer/tests/data/samegame.qzt)
- [samegame.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.qml)
- [samegame.js](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.js)
- [Block.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Block.qml)
- [Button.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Button.qml)

### 必要なコマンド

- `cmake`
- `qmlprofileranalyzer`
- 必要なら `Qt Creator`

### ビルド

まだビルドしていない場合は次を実行する。

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/usr/local/qt/Qt/6.11.0/macos
cmake --build build
```

生成される実行ファイル:

- `build/qmlprofileranalyzer`

## 手順 1. `.qzt` の全体統計を確認

```bash
./build/qmlprofileranalyzer summary <trace.qzt>
```

確認する項目:

- `file format`
- `traceDuration`
- `eventTypes`
- `events`
- `ranges`
- `pointEvents`
- `notes`
- `longestRange`
- `per-type`

今回の観察例:

- `file format: qzt/binary`
- `traceDuration: 20455310588`
- `eventTypes=126`
- `events=10097`
- `ranges=7433`
- `pointEvents=2664`
- `notes=0`
- `longestRange: idx=26 type=samegame.qml:65 duration=86627292`

ここで分かること:

- トレース全体の規模
- range と point event の比率
- 最も重い処理の入口
- 後で確認すべき `per-type` 候補

## 手順 2. type 情報をコード位置と結び付ける

```bash
./build/qmlprofileranalyzer types <trace.qzt>
```

一般に確認する項目:

- `type=Creating`
- `type=Binding`
- `type=Javascript`
- `type=HandlingSignal`
- `type=PixmapCache`
- `details=...`
- `file=...:line:column`

同じ `.qzt` でも、注目すべき代表項目は対象アプリに依存する。

samegame での例:

- `samegame.qml:8`
- `samegame.qml:19`
- `samegame.qml:46`
- `samegame.qml:65`
- `samegame.js:33`
- `samegame.js:65`
- `samegame.js:82`
- `samegame.js:110`
- `Block.qml:16`
- `Button.qml:35`

ここで、行番号と `details` をソースコードへ対応付ける。

## 手順 3. `events-aggregated` 出力で高水準集約を確認

```bash
./build/qmlprofileranalyzer events-aggregated <trace.qzt> | sed -n '1,24p'
```

集約対象を含む `.qzt` の場合は、先頭に `# aggregates` が出ることを確認する。
画像読込や scene graph、memory allocation が無いトレースでは、この節は参考扱いでよい。

今回の観察例:

```text
# aggregates
4670477111  aggregate=PixmapCacheLoad  type=background.jpg:0  duration=18490692  file=background.jpg  width=909 height=600
7963661406  aggregate=PixmapCacheLoad  type=redStone.png:0  duration=211723  file=redStone.png  width=40 height=40
7963989359  aggregate=PixmapCacheLoad  type=blueStone.png:0  duration=99380  file=blueStone.png  width=40 height=40
7964621528  aggregate=PixmapCacheLoad  type=greenStone.png:0  duration=113709  file=greenStone.png  width=40 height=40
```

一般にここで分かること:

- 画像読込系イベントの開始時刻
- 読込完了までの duration
- ファイル名
- 幅 / 高さ
- `SceneGraph` のフレーム種別ごとの totalTiming
- `MemoryAllocation` の種別ごとの件数と累積量

samegame では `background.jpg` の行が、
Qt Creator で見える `読込み済み画像` に対応する。

## 手順 4. Qt Creator 側と比較する

Qt Creator で `.qzt` を開く場合は、`Load QML Trace` から読み込む。

一般に確認する代表項目:

- 選択したイベント名
- ファイル名
- duration
- 必要なら幅 / 高さ、テキスト、量などの付随情報

samegame での例:

- `読込み済み画像`
- `ファイル: background.jpg`
- `幅: 909 px`
- `高さ: 600 px`
- `持続時間: 18.5 ms`

`qmlprofileranalyzer` 側では、samegame の場合は次の raw event と集約行が対応する。

- `4670477111  idx=79  type=background.jpg:0`
  - `cacheEventType=3`
- `4688967803  idx=80  type=background.jpg:0  height=600 width=909`
  - `cacheEventType=0`
- `4688967803  idx=81  type=background.jpg:0`
  - `cacheEventType=4`
- `4670477111  aggregate=PixmapCacheLoad  type=background.jpg:0  duration=18490692  file=background.jpg  width=909 height=600`

比較の観点:

- イベント名や意味が一致しているか
- ファイル名が一致しているか
- duration が概ね一致しているか
- 必要なら幅 / 高さや補助属性が一致しているか

## 手順 5. 重い処理を拾う

`summary` の `per-type` を見て、大きい `totalDuration` と `maxDuration` を拾う。

一般に拾う対象:

- 最長 `maxDuration`
- 大きい `totalDuration`
- `count` が多く合計時間も大きい `Javascript`
- 繰り返し出る `Binding`
- 初期化時に集中している `Creating`

samegame での観察例:

- `samegame.qml:65`
  - `Creating`
  - `QtQuick/Text`
  - `maxDuration=86627292`
- `samegame.qml:8`
  - `Creating`
  - `QtQuick/Rectangle`
  - `totalDuration=30315700`
- `samegame.qml:46`
  - `Creating`
  - `Dialog.qml`
  - `totalDuration=30056009`
- `Dialog.qml:7`
  - `Creating`
  - `QtQuick/Rectangle`
  - `totalDuration=30041217`
- `Dialog.qml:23`
  - `Creating`
  - `QtQuick/Text`
  - `totalDuration=30008406`
- `samegame.js:33`
  - `Javascript`
  - `createBlock`
  - `count=204`
  - `totalDuration=16898553`

解釈の基本:

- `Creating` が大きいものは初期 UI 生成コスト
- `Javascript` が多いものはゲームロジックや更新処理
- `count` が多いものは繰り返し処理

## 手順 6. 軽い処理を拾う

`per-type` の中から、回数はあっても 1 回あたりが小さいものを拾う。

samegame での観察例:

- `samegame.js:65`
  - `handleClick`
  - `count=9`
  - `totalDuration=2376956`
- `samegame.js:110`
  - `shuffleDown`
  - `count=9`
  - `totalDuration=1197271`
- `Block.qml:16`
  - `source` の binding / javascript
  - 件数は多いが 1 回あたりは小さい

## 手順 7. ソースコードへ対応付ける

分析で拾った行番号を、実際の対象ソースへ対応付ける。

一般に見るファイル:

- メイン QML
- 関連する QML component
- 関連する JS
- 必要なら C++ 側の起動コード

samegame の場合:

- [samegame.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.qml)
- [samegame.js](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/samegame.js)
- [Block.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Block.qml)
- [Button.qml](/usr/local/qt/Qt/Examples/Qt-6.10.2/quick/tutorials/samegame/samegame3/Button.qml)

対応付けの例:

- `samegame.qml:65`
  - `Text` 作成
- `samegame.qml:19`
  - 背景 `Image`
- `samegame.qml:46`
  - `Dialog.qml` の生成
- `samegame.js:33`
  - `createBlock`
- `samegame.js:82`
  - `floodFill`
- `samegame.js:110`
  - `shuffleDown`
- `Block.qml:16`
  - `source` binding

## 手順 8. 評価文へ落とす観点

評価文には少なくとも次を入れる。

1. 全体評価
- `.qzt` が正常に読めるか
- Qt Creator と比較可能か

2. 主要な観察点
- トレース全体量
- 最長 range
- 次に大きい処理
- `PixmapCache` の代表例

3. 解釈
- 初期 UI 生成とゲームロジックのどちらが重いか
- Qt Creator とどこが一致しているか
- まだどこが違うか

4. コード単位の見立て
- 比較的軽い箇所
- 相対的に重い箇所

5. 結論
- `.qzt` reader の妥当性
- Qt Creator 比較用サンプルとしての有用性

samegame の具体例としてまとめたものが
[SAMEGAME_QZT_EVALUATION.md](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/SAMEGAME_QZT_EVALUATION.md)
である。

## 既知の注意点

### 1. ソースと `.qzt` は対応系統である必要がある

- 行番号が変わった samegame ソースを使うと、評価結果の対応付けがずれる
- 一般にも、別バージョンのソースを使うと対応付けがずれる
- samegame の今回の基準は `Qt-6.10.2/quick/tutorials/samegame/samegame3` である

### 2. Qt Creator と `qmlprofileranalyzer` の表示モデルは同じではない

- Qt Creator は高水準に集約して見せる
- `qmlprofileranalyzer` は raw event を主に出す
- `PixmapCache` だけは比較しやすいように集約行を足している

### 3. 評価文は自動生成ではない

- `summary`
- `types`
- `events`
- Qt Creator での確認
- ソースコード対応付け

を人間が読んでまとめる手順になっている。

## 関連文書

- [SAMEGAME_QZT_EVALUATION.md](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/SAMEGAME_QZT_EVALUATION.md)
- [STATUS.md](/Users/sugita/src/qt/tools/qmlprofileranalyzer/docs/STATUS.md)
- [README.md](/Users/sugita/src/qt/tools/qmlprofileranalyzer/README.md)
