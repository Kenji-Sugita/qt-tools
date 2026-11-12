# 再生成乖離の立て直し計画

この文書は、現時点で行うべき作業を具体化した計画である。
目的は、AI 実装で積み上がった判断を正本へ戻し、仕様・設計契約・テスト・生成コードの
乖離を小さくすることである。

## 現状認識

- `source_memo.md` は、現行ワークフローで三点セットを生成するための仕様化前ノートである。
- 三点セットは、利用者視点・UI 視点・業務ロジック視点の仕様を表す生成物である。
- 現在のワークフローでは、三点セットを手作業で直接修正するのではなく、
  `source_memo.md` から仕様化ワークフローで再生成・更新する前提で扱う。
- `implementation_constraints.md` は、OS、Qt、コンパイラ差分など横断的な実装制約の正本である。
- `implementation_design.md` は、現在実装に近いコードを再生成するための実装設計契約の正本である。
- 直近のフル機能再生成比較では、既存テストは通るが `src/`、`include/`、`CMakeLists.txt` に
  大きな差分が出た。
- これは、仕様に機能が書かれていても、実装時に AI が埋めた設計判断が仕様・設計契約へ
  逆流していなかったためである。

## まず守る方針

- 三点セットを直接手編集しない。
- 現行ワークフローでは、三点セットへ反映させるための入力先は `source_memo.md` である。
- `source_memo.md` だけでは毎回同じ観点が落ちる場合は、個別アプリの入力ではなく
  app-generation-workflow 側のテンプレートまたはプロンプトへ戻す。
- 実装制約を三点セットへ混ぜない。
- 実装設計契約を三点セットへ混ぜない。
- コード再生成で消えると困る判断は、コードだけに残さない。
- 実装後の逆流工程は、コミット前レビューの一部として必ず行う。

## 文書の役割分担

| ファイル | 役割 |
| --- | --- |
| `source_memo.md` | 三点セットを生成・更新するための仕様化前ノート |
| `specs/01_usecase_spec.md` | 利用者、目的、ユースケース |
| `specs/02_ui_spec.md` | 画面、操作、表示、UI 文言、ショートカット |
| `specs/03_business_spec.md` | 業務ロジック、入力検証、保存対象、制限値 |
| `implementation_constraints.md` | 環境依存、Qt/OS/コンパイラ差分、横断実装制約 |
| `implementation_design.md` | クラス責務、状態遷移、保存 JSON、Undo 粒度、再生成維持契約 |
| `tests/` | 維持すべき振る舞いを機械的に固定する検証 |
| `prompts/implementation_prompt.md` | 実装時に参照すべき入力ファイルの束ね役 |

## 直ちに行う作業

### 1. 現在実装から実装判断を棚卸しする

対象ファイル:

- `include/BoardModel.h`
- `include/CanvasWidget.h`
- `include/MainWindow.h`
- `include/DrawingElement.h`
- `include/BoardSettings.h`
- `src/BoardModel.cpp`
- `src/CanvasWidget.cpp`
- `src/MainWindow.cpp`
- `src/DrawingElement.cpp`
- `src/BoardSettings.cpp`
- `src/PlatformColorDialog.cpp`
- `CMakeLists.txt`

確認する観点:

- クラス責務として維持すべきものは何か。
- UI から見える仕様として、`source_memo.md` へ戻して仕様化ワークフローで
  三点セットへ反映させるべきものは何か。
- OS/Qt 差分として `implementation_constraints.md` へ記録すべきものは何か。
- 再生成時に構造を維持するため `implementation_design.md` へ記録すべきものは何か。
- テストで固定すべき挙動は何か。
- 単なる実装表現で、将来変わってもよいものは何か。

成果物:

- `implementation_design.md` の不足追記。
- `implementation_constraints.md` の不足追記。
- `source_memo.md` へ戻して仕様化ワークフローで三点セットへ反映させるべき要求の一覧。
- 追加すべきテストの一覧。

### 2. `implementation_design.md` を精査する

すでに作成済みの `implementation_design.md` を、現在コードと照合して精査する。

必ず確認する項目:

- `BoardModel` の公開 API が維持されているか。
- `CanvasWidget` の `DragMode` と一時状態が維持されているか。
- 折れ線、ベジェ曲線、円弧、テキスト編集の状態遷移が十分に書かれているか。
- Undo/Redo のスナップショット粒度が十分に書かれているか。
- 保存 JSON のキーが現在実装と一致しているか。
- ツールバーのグループ順、ボタン、ショートカットが現在実装と一致しているか。
- Windows の `QColorDialog` 回避策が `PlatformColorDialog` に閉じていることが明記されているか。
- `CMakeLists.txt` のターゲット構成が現在実装と一致しているか。

判断基準:

- 再生成で同じ構造を維持したい内容は残す。
- 将来のリファクタを不必要に縛る細部は書かない。
- ユーザーから見える仕様は `implementation_design.md` だけに閉じ込めず、
  `source_memo.md` へ戻し、仕様化ワークフローで三点セットへ反映させる。

### 3. 三点セットへ反映させたい内容を `source_memo.md` へ戻す

三点セットを直接編集するのではなく、まず反映させたい内容を一覧化し、
このアプリ固有の内容は `source_memo.md` へ戻す。

現時点の whiteboard ワークフローでは、三点セットを再生成・更新するための
アプリ固有入力は `source_memo.md` である。したがって、三点セットへ反映させたい
whiteboard 固有の要求は `source_memo.md` に書く。

ただし、毎回どのアプリでも必要になる仕様化観点、例えば「実装後に AI が補った
ユーザー可視仕様を仕様化入力へ戻す候補として整理する」などは、個別の
`source_memo.md` ではなく、app-generation-workflow 側のテンプレートや
プロンプトへ戻す。

戻す対象の例:

- ユーザー操作として見えるもの。
- 表示や見た目としてユーザーが認識するもの。
- ショートカットやメニュー構成。
- 保存対象や入力制限。
- Undo/Redo のユーザーから見える単位。
- ページ数制限やズーム制限。

戻さない対象の例:

- `BoardModel` の private メンバ構成。
- `CanvasWidget` の private enum 名。
- `QPainterPathStroker` を使うかどうかの実装詳細。
- `QAction` をどのローカル変数名で作るか。
- Windows 固有の `QColorDialog` 回避策。

成果物:

- `source_memo.md` へ戻して三点セットへ反映させるべき項目一覧。
- `source_memo.md` へ戻すための追記案。
- app-generation-workflow 側へ戻すべき共通ルールがある場合は、その追記案。

### 4. `source_memo.md` の位置づけを明確化する

`source_memo.md` は仕様化前ノートであり、現行ワークフローでは三点セットを
生成・更新するための入力である。最新仕様の正本は三点セットだが、三点セットを
直接手編集しないため、三点セットへ反映させたい whiteboard 固有の要求は
`source_memo.md` に戻す。

推奨構成:

```md
## 未反映・検討中

## 反映済み

## 判断履歴

## 実装後に仕様へ戻す候補
```

運用:

- 新しい whiteboard 固有の要求はまず `source_memo.md` の `未反映・検討中` に入れる。
- 仕様化ワークフローで三点セットへ反映されたら `反映済み` へ移す。
- 実装中に AI が補った判断は `実装後に仕様へ戻す候補` に一度記録する。
- 維持しない判断は削除または `判断履歴` に理由付きで残す。
- 複数アプリに共通する運用ルールや仕様化観点は `source_memo.md` に閉じ込めず、
  app-generation-workflow 側へ戻す。

### 5. テスト不足を洗い出す

`implementation_design.md` に書くだけでは、再生成時に見た目や操作が崩れても検出できない。
壊れると困る挙動はテストへ落とす。

追加候補:

- ツールバーのボタン順とグループ構成。
- フォントボタンが Text の右横にあること。
- 線幅 0 でテキストが描画されること。
- 選択枠が要素 bounds の外側に描かれること。
- ラバーバンド中に選択状態が更新されること。
- Ctrl/Meta+クリックで選択追加されること。
- 複数選択時の z-order 移動で相対順が維持されること。
- ベジェ曲線の端点延長で始点側 prepend、終点側 append になること。
- 折れ線延長中に反対側端点クリックで重複点を追加せず closed になること。
- 保存 JSON に既存キーが維持されること。
- Windows では `PlatformColorDialog` の初期色補正が UI 側に重複しないこと。

### 6. フル再生成比較を再実行する

`implementation_design.md` と必要な入力整理後に、再度フル機能再生成比較を行う。

比較手順:

1. `/tmp` に新しいスクラッチディレクトリを作る。
2. `source_memo.md`、三点セット、`implementation_constraints.md`、
   `implementation_design.md`、`prompts/implementation_prompt.md` を入力として配置する。
3. `include/`、`src/`、`tests/`、`CMakeLists.txt` を再生成する。
4. CMake configure を実行する。
5. build を実行する。
6. `ctest --output-on-failure` を実行する。
7. 現在コードと `diff -qr` で比較する。
8. `git diff --no-index --stat` で差分規模を確認する。

判定:

- テストが落ちる場合は、機能再現が不足している。
- テストは通るが差分が大きい場合は、設計契約またはテストの固定が不足している。
- `MainWindow.cpp` と `CanvasWidget.cpp` が大きく短縮される場合は、UI 構成または状態遷移契約がまだ不足している。
- 保存 JSON のキーが変わる場合は、互換仕様の固定が不足している。

### 7. 差分を分類して戻す

再生成差分は次の 4 種類に分類する。

| 分類 | 意味 | 対応 |
| --- | --- | --- |
| 機能仕様差分 | ユーザーから見える機能が違う | `source_memo.md` へ戻し、仕様化ワークフローで三点セットへ反映させる |
| 実装設計契約差分 | クラス責務、状態遷移、保存形式が違う | `implementation_design.md` へ戻す |
| 実装制約差分 | OS/Qt/コンパイラ差分対応が消える | `implementation_constraints.md` へ戻す |
| 表現差分 | 振る舞いも設計契約も同じで書き方だけ違う | 原則そのまま。必要なら許容差分として記録 |

## コミット前チェック

今後の変更では、コミット前に次を確認する。

- 実装中に仕様にない判断を AI が補っていないか。
- 補った判断は今後も維持すべきか。
- 維持する場合、どの正本へ戻したか。
- ユーザーから見える whiteboard 固有の変更は `source_memo.md` へ戻す経路に乗っているか。
- 複数アプリに共通する変更は app-generation-workflow 側へ戻す経路に乗っているか。
- 実装構造を維持したい変更は `implementation_design.md` に反映したか。
- 環境依存対応は `implementation_constraints.md` に反映したか。
- 壊れると困る挙動はテストにしたか。
- コード再生成で消えると困る修正をコードだけに残していないか。
- build と test が通っているか。

## 優先順位

### 最優先

1. `implementation_design.md` と現在コードの照合。
2. 三点セットへ反映させたいユーザー可視仕様を `source_memo.md` へ戻すための一覧化。
3. 保存 JSON と Undo/Redo 粒度のテスト追加候補整理。
4. フル再生成比較の再実行。

### 次点

1. `source_memo.md` を「未反映」「反映済み」「判断履歴」へ整理する。
2. ツールバー構成のテスト追加。
3. Canvas 状態遷移のテスト追加。
4. 再生成差分の許容基準を文書化する。

### 後回しでよいもの

1. 完全なコード一致を目指すこと。
2. private 変数名やローカル変数名の一致。
3. 将来変わってもよい実装表現の固定。

## 完了条件

この立て直し作業は、次の状態になれば完了とする。

- `implementation_design.md` が現在実装の維持すべき設計判断を十分に表している。
- 三点セットへ反映させたい whiteboard 固有のユーザー可視仕様が一覧化され、
  `source_memo.md` へ戻す準備ができている。
- 複数アプリへ共通化すべき仕様化観点がある場合、app-generation-workflow 側へ
  戻す準備ができている。
- `implementation_constraints.md` に環境依存制約が集約されている。
- 再生成コードが build と test を通る。
- 再生成差分を、機能仕様差分、実装設計契約差分、実装制約差分、表現差分に分類できる。
- コードだけに残っている重要な実装判断がない。
