# Samegame Improvement Proposal

## 目的

この文書は、`tests/data/samegame.qzt` の解析結果を根拠に、
samegame の改善案を整理した提案書である。

この作業では samegame のコードは変更しない。
対象は改善案の整理と優先順位付けのみとする。

## 根拠

参照した入力は次。

- `tests/data/samegame.qzt`
- `./build/qmlprofileranalyzer summary tests/data/samegame.qzt`
- `./build/qmlprofileranalyzer types tests/data/samegame.qzt`
- `./build/qmlprofileranalyzer events-aggregated tests/data/samegame.qzt`
- Qt samegame サンプルのソース

主な観察結果は次。

- `traceDuration`: `20455310588 ns`
- `eventTypes`: `126`
- `events`: `10097`
- `ranges`: `7433`
- `pointEvents`: `2664`
- 最長 range: `samegame.qml:65`
  - `Creating`
  - `QtQuick/Text`
  - `86627292 ns`
- 大きい初期化コスト:
  - `samegame.qml:46` `Dialog.qml` 作成 `30056009 ns`
  - `Dialog.qml:7` `QtQuick/Rectangle` 作成 `30040444 ns`
  - `Dialog.qml:23` `QtQuick/Text` 作成 `29997528 ns`
  - `samegame.qml:19` 背景 `Image` 作成 `18583961 ns`
- 大きいロジック処理:
  - `samegame.js:33` `createBlock` `204` 回 合計 `16898553 ns`
  - `samegame.js:82` `floodFill` `329` 回 合計 `3550235 ns`
  - `samegame.js:65` `handleClick` `9` 回 合計 `2376956 ns`
  - `samegame.js:110` `shuffleDown` `9` 回 合計 `1197271 ns`
- 画像読込:
  - `background.jpg` 読込約 `18.49 ms`
  - `redStone.png`, `blueStone.png`, `greenStone.png` は初回読込後にキャッシュ参照

## 前提

- 既存のゲーム仕様は維持する
- 見た目変更は最小限にとどめる
- まずは起動時の負荷とクリック応答の改善を優先する
- 効果見込みは `samegame.qzt` の 1 トレースに基づく

## 優先順位

1. 起動時に不要な UI の遅延生成
2. 背景画像読込コストの削減
3. 盤面生成コストの削減
4. クリック時探索処理の効率化

## 改善案

### 1. Dialog を遅延生成する

対象箇所:

- `samegame.qml:46`
- `Dialog.qml:7`
- `Dialog.qml:23`

内容:

- 起動時に `Dialog` を即生成せず、必要になった時点で生成する
- 手段としては `Loader` による遅延生成が第一候補

必要性:

- `Dialog` 関連の `Creating` が約 `30 ms` 級でまとまっており、
  起動時コストの大きな割合を占めている
- ダイアログは通常、ゲームオーバー時まで不要

期待効果:

- 初期表示時間の短縮
- 起動直後の `SceneGraphPolishAndSync` 負荷の一部緩和

既存挙動への影響:

- 初回の `dialog.show()` 時にだけ生成コストが後ろへ移る
- 表示内容とゲーム仕様は維持可能

実装コスト:

- 低から中

優先度:

- 高

### 2. 背景画像の負荷を下げる

対象箇所:

- `samegame.qml:19`
- `background.jpg`

内容:

- 背景画像の解像度またはファイルサイズを見直す
- 必要なら表示サイズに近い画像へ差し替える
- 代替として、初回表示に不要なら読込タイミングの調整も検討する

必要性:

- `background.jpg` の読込が約 `18.49 ms`
- 起動時の目立つ単発コストになっている

期待効果:

- 起動時の画像 I/O とデコード時間の削減
- 初期描画までの待ち時間短縮

既存挙動への影響:

- 画質や見た目に軽微な差が出る可能性がある
- レイアウトやゲーム仕様への影響はない

実装コスト:

- 低

優先度:

- 高

### 3. Block 生成の固定コストを下げる

対象箇所:

- `samegame.js:33` `createBlock`
- `Block.qml:12`
- `Block.qml:16`

内容:

- `createObject()` を大量に呼ぶ構成を見直す
- 段階案は次の 2 つ
  - 最小変更案: `source` binding をやめ、生成時に画像 URL を直接設定する
  - 効果優先案: `Repeater` または model ベースに寄せて生成責務を QML 側へ移す

必要性:

- `createBlock` が `204` 回で合計約 `16.9 ms`
- `Block.qml` の `Image.source` が JS 式 binding になっており、生成時評価が多い

期待効果:

- 新規ゲーム開始時の盤面生成時間短縮
- 小さい割当てと binding 評価回数の削減

既存挙動への影響:

- 最小変更案なら仕様影響はほぼない
- model 化は構造変更になるため、挙動回帰確認が必要

実装コスト:

- 最小変更案は低から中
- model 化は中から高

優先度:

- 中から高

### 4. floodFill の一時領域確保と再帰処理を見直す

対象箇所:

- `samegame.js:82` `floodFill`
- `samegame.js:92` `new Array(maxIndex)`

内容:

- 毎回の `new Array(maxIndex)` をやめて作業バッファを再利用する
- 必要なら再帰をスタックベースの反復処理に置き換える

必要性:

- `floodFill` は `329` 回で合計約 `3.55 ms`
- 1 回ごとのコストは小さいが、クリック時の主処理として積み上がる
- 小さい割当てが多いトレース傾向とも整合する

期待効果:

- クリック応答の改善
- メモリ割当て回数の削減
- 盤面が大きくなった場合の再帰深度リスク低下

既存挙動への影響:

- ルール自体は維持可能
- ロジック変更のため、消去条件とスコア計算の回帰確認が必要

実装コスト:

- 中

優先度:

- 中

### 5. shuffleDown のインデックス計算を整理する

対象箇所:

- `samegame.js:110`

内容:

- `index(column, row)` の重複計算を減らす
- 列ごとの走査で参照位置を局所変数に持ち、配列アクセス回数を抑える

必要性:

- 現状の支配的コストではないが、クリック後に毎回走る処理
- `handleClick` 系の累積時間を詰める余地がある

期待効果:

- クリック後処理の微改善
- コードの追跡しやすさ向上

既存挙動への影響:

- 仕様影響は基本的にない

実装コスト:

- 低から中

優先度:

- 低から中

## 実施順の推奨

最小リスクで進めるなら次の順がよい。

1. `Dialog` 遅延生成
2. 背景画像の軽量化
3. `floodFill` の作業配列再利用
4. `Block` 生成の最小変更案

この順なら、仕様影響の少ないものから先に効果を取りにいける。

## 2 つの実施方針

### 方針 A: 最小変更で効果を取る

対象:

- `Dialog` 遅延生成
- 背景画像軽量化
- `floodFill` 作業配列再利用
- `Block` の `source` 直接設定

向いている場合:

- サンプルの分かりやすさを維持したい
- チュートリアルコードとしての構造を大きく変えたくない

### 方針 B: 効果優先で構造も見直す

対象:

- 方針 A の全項目
- `Block` 生成の model 化
- ロジック処理の反復化

向いている場合:

- 教材よりも実行性能を優先したい
- 多少の構造変更を許容できる

## 追加で確認したいこと

- `Dialog` は実際に起動直後から必要か
- 背景画像の見た目品質にどこまで制約があるか
- チュートリアル性を優先するか、性能改善を優先するか
- 改善後に新しいトレースを取り、起動時とクリック時を分けて再評価できるか

## 結論

`samegame.qzt` からは、改善余地は主に 2 系統ある。

- 起動時の UI 構築と画像読込
- ゲーム開始時およびクリック時の JS / オブジェクト生成

最初に着手すべきなのは `Dialog` の遅延生成と背景画像の軽量化である。
次に `floodFill` の作業配列再利用、最後に `Block` 生成方式の見直しを検討するのが妥当である。

## 想定 diff

以下は実装時のイメージを共有するための想定 diff である。
この文書作成時点では samegame 本体には適用していない。

### 最小変更案

目的:

- 既存のサンプル構造を大きく変えずに効果を取る
- 起動時コストとクリック時コストを小さめの変更で削る

含める変更:

- `Dialog` 遅延生成
- 背景画像読込負荷の低減
- `Block` の `source` を生成時に直接設定
- `floodFill` の作業配列再利用
- `shuffleDown` の軽微な整理

#### 1. `Dialog` 遅延生成と背景画像の読込負荷低減

```diff
diff --git a/samegame.qml b/samegame.qml
--- a/samegame.qml
+++ b/samegame.qml
@@
         Image {
             id: background
             anchors.fill: parent
             source: "pics/background.jpg"
+            sourceSize.width: width
+            sourceSize.height: height
             fillMode: Image.PreserveAspectCrop
         }
@@
-    Dialog {
-        id: dialog
-        anchors.centerIn: parent
-        z: 100
-    }
+    Loader {
+        id: dialogLoader
+        anchors.centerIn: parent
+        z: 100
+        active: false
+        source: "Dialog.qml"
+    }
```

補足:

- `sourceSize` は表示サイズに近いサイズでの読込を促す案
- `Dialog` は `Loader` に置き換え、必要になるまで生成しない

#### 2. `dialog.show()` 呼び出し側の調整

```diff
diff --git a/samegame.js b/samegame.js
--- a/samegame.js
+++ b/samegame.js
@@
-    dialog.hide();
+    if (dialogLoader.item)
+        dialogLoader.item.hide();
@@
-        dialog.show("Game Over. Your score is " + gameCanvas.score);
+        if (!dialogLoader.active)
+            dialogLoader.active = true;
+        dialogLoader.item.show("Game Over. Your score is " + gameCanvas.score);
```

補足:

- `Dialog` の実体が未生成でも安全に扱えるようにする
- 初回 `show()` 時に `Loader` を有効化する

#### 3. `Block` 生成時に画像 URL を直接設定する案

```diff
diff --git a/Block.qml b/Block.qml
--- a/Block.qml
+++ b/Block.qml
@@
 Item {
     id: block
 
     property int type: 0
+    property url source: ""
@@
     Image {
         id: img
 
         anchors.fill: parent
-        source: {
-            if (type == 0)
-                return "pics/redStone.png";
-            else if (type == 1)
-                return "pics/blueStone.png";
-            else
-                return "pics/greenStone.png";
-        }
+        source: block.source
     }
 }
```

```diff
diff --git a/samegame.js b/samegame.js
--- a/samegame.js
+++ b/samegame.js
@@
 var maxColumn = 10;
 var maxRow = 15;
 var maxIndex = maxColumn * maxRow;
 var board = new Array(maxIndex);
 var component;
+var blockSources = [
+    "pics/redStone.png",
+    "pics/blueStone.png",
+    "pics/greenStone.png"
+];
@@
-        dynamicObject.type = Math.floor(Math.random() * 3);
+        dynamicObject.type = Math.floor(Math.random() * blockSources.length);
+        dynamicObject.source = blockSources[dynamicObject.type];
         dynamicObject.x = column * gameCanvas.blockSize;
         dynamicObject.y = row * gameCanvas.blockSize;
```

補足:

- `Block.qml` 側の JS 式 binding を削減する最小変更案
- 生成ごとの式評価を避け、生成時に確定値を渡す

#### 4. `floodFill` の作業配列再利用と反復化

```diff
diff --git a/samegame.js b/samegame.js
--- a/samegame.js
+++ b/samegame.js
@@
 var fillFound; //Set after a floodFill call to the number of blocks found
 var floodBoard; //Set to 1 if the floodFill reaches off that node
+var floodMark = 0;
@@
-function floodFill(column, row, type) {
-    if (board[index(column, row)] == null)
-        return;
-    var first = false;
-    if (type == -1) {
-        first = true;
-        type = board[index(column, row)].type;
-
-        //Flood fill initialization
-        fillFound = 0;
-        floodBoard = new Array(maxIndex);
-    }
-    if (column >= maxColumn || column < 0 || row >= maxRow || row < 0)
-        return;
-    if (floodBoard[index(column, row)] == 1 || (!first && type != board[index(column, row)].type))
-        return;
-    floodBoard[index(column, row)] = 1;
-    floodFill(column + 1, row, type);
-    floodFill(column - 1, row, type);
-    floodFill(column, row + 1, type);
-    floodFill(column, row - 1, type);
-    if (first == true && fillFound == 0)
-        return;     //Can't remove single blocks
-    board[index(column, row)].opacity = 0;
-    board[index(column, row)] = null;
-    fillFound += 1;
+function floodFill(column, row, type) {
+    if (column >= maxColumn || column < 0 || row >= maxRow || row < 0)
+        return;
+
+    var startIndex = index(column, row);
+    var startObject = board[startIndex];
+    if (startObject == null)
+        return;
+
+    if (!floodBoard || floodBoard.length !== maxIndex)
+        floodBoard = new Array(maxIndex);
+
+    fillFound = 0;
+    floodMark += 1;
+    var targetType = (type === -1) ? startObject.type : type;
+    var stack = [[column, row]];
+    var pending = [];
+
+    while (stack.length > 0) {
+        var cell = stack.pop();
+        var currentColumn = cell[0];
+        var currentRow = cell[1];
+        if (currentColumn >= maxColumn || currentColumn < 0 || currentRow >= maxRow || currentRow < 0)
+            continue;
+
+        var currentIndex = index(currentColumn, currentRow);
+        var currentObject = board[currentIndex];
+        if (currentObject == null || floodBoard[currentIndex] === floodMark || currentObject.type !== targetType)
+            continue;
+
+        floodBoard[currentIndex] = floodMark;
+        pending.push(currentIndex);
+        stack.push([currentColumn + 1, currentRow]);
+        stack.push([currentColumn - 1, currentRow]);
+        stack.push([currentColumn, currentRow + 1]);
+        stack.push([currentColumn, currentRow - 1]);
+    }
+
+    if (pending.length <= 1)
+        return;
+
+    for (var i = 0; i < pending.length; ++i) {
+        var removeIndex = pending[i];
+        board[removeIndex].opacity = 0;
+        board[removeIndex] = null;
+    }
+    fillFound = pending.length;
 }
```

補足:

- `new Array(maxIndex)` の毎回確保を避ける
- 再帰を反復処理へ置き換え、盤面が大きくなった場合も扱いやすくする
- 単独ブロックを消さない既存ルールは `pending.length <= 1` で維持する

#### 5. `shuffleDown` の参照回数を減らす案

```diff
diff --git a/samegame.js b/samegame.js
--- a/samegame.js
+++ b/samegame.js
@@
 function shuffleDown() {
     //Fall down
     for (var column = 0; column < maxColumn; column++) {
         var fallDist = 0;
         for (var row = maxRow - 1; row >= 0; row--) {
-            if (board[index(column, row)] == null) {
+            var currentIndex = index(column, row);
+            var currentObject = board[currentIndex];
+            if (currentObject == null) {
                 fallDist += 1;
             } else {
                 if (fallDist > 0) {
-                    var obj = board[index(column, row)];
-                    obj.y += fallDist * gameCanvas.blockSize;
-                    board[index(column, row + fallDist)] = obj;
-                    board[index(column, row)] = null;
+                    currentObject.y += fallDist * gameCanvas.blockSize;
+                    board[index(column, row + fallDist)] = currentObject;
+                    board[currentIndex] = null;
                 }
             }
         }
     }
```

補足:

- 支配的な改善ではないが、クリック後に毎回走る処理の無駄を少し減らせる
- 読みやすさ改善にも寄与する

### 効果優先案

目的:

- 初期化と盤面生成の固定コストをさらに下げる
- 多少の構造変更を許容して改善幅を広げる

含める変更:

- 最小変更案の全項目
- `Block` 生成の model 化
- 盤面ロジックの反復処理化

#### 1. `Block` 生成を model ベースへ寄せる案

```diff
diff --git a/samegame.qml b/samegame.qml
--- a/samegame.qml
+++ b/samegame.qml
@@
         Item {
             id: gameCanvas
 
             property int score: 0
             property int blockSize: 40
+            property var boardModel: []
@@
             MouseArea {
                 anchors.fill: parent
                 onClicked: (mouse)=> SameGame.handleClick(mouse.x, mouse.y)
             }
+
+            Repeater {
+                model: gameCanvas.boardModel
+
+                delegate: Block {
+                    required property int index
+                    required property int type
+                    x: (index % SameGame.maxColumn) * gameCanvas.blockSize
+                    y: Math.floor(index / SameGame.maxColumn) * gameCanvas.blockSize
+                    width: gameCanvas.blockSize
+                    height: gameCanvas.blockSize
+                    source: SameGame.blockSources[type]
+                    visible: type >= 0
+                }
+            }
         }
```

```diff
diff --git a/samegame.js b/samegame.js
--- a/samegame.js
+++ b/samegame.js
@@
-var board = new Array(maxIndex);
+var board = new Array(maxIndex);
+var blockSources = [
+    "pics/redStone.png",
+    "pics/blueStone.png",
+    "pics/greenStone.png"
+];
@@
     board = new Array(maxIndex);
+    gameCanvas.boardModel = new Array(maxIndex);
     gameCanvas.score = 0;
     for (var column = 0; column < maxColumn; column++) {
         for (var row = 0; row < maxRow; row++) {
-            board[index(column, row)] = null;
-            createBlock(column, row);
+            var cellIndex = index(column, row);
+            var blockType = Math.floor(Math.random() * blockSources.length);
+            board[cellIndex] = blockType;
+            gameCanvas.boardModel[cellIndex] = { index: cellIndex, type: blockType };
         }
     }
```

補足:

- `createObject()` の大量呼び出しを避ける方向の案
- 教材コードとしてはやや構造変更が大きい

#### 2. 盤面状態を object 参照から軽量データへ寄せる案

```diff
diff --git a/samegame.js b/samegame.js
--- a/samegame.js
+++ b/samegame.js
@@
-    if (board[index(column, row)] == null)
+    if (board[index(column, row)] < 0)
         return;
@@
-        type = board[index(column, row)].type;
+        type = board[index(column, row)];
@@
-        if (currentObject == null || floodBoard[currentIndex] === floodMark || currentObject.type !== targetType)
+        if (currentObject < 0 || floodBoard[currentIndex] === floodMark || currentObject !== targetType)
             continue;
@@
-        board[removeIndex].opacity = 0;
-        board[removeIndex] = null;
+        board[removeIndex] = -1;
+        gameCanvas.boardModel[removeIndex].type = -1;
```

補足:

- JS ロジックが QML object 参照を直接持たない構造へ寄せる案
- 性能面では有利だが、UI 同期方法を含めた見直しが必要
