# Qt Tools

Qt/C++ の開発支援ツール、対話的プログラミング環境、資料作成ツール、
アプリ開発の手順と文書テンプレート、作業コンテキストを管理するテンプレート、
日常的に使用するユーティリティーなどをまとめたリポジトリーです。

Qt を使ったアプリケーションやライブラリーを中心に、
Python/Bash によるツールや、生成 AI を利用した Qt 開発を支援するための
ファイルも含まれています。


## Featured: Interactive Qt/C++

C++/Qt を、ビルドして実行するだけではなく、
対話的に試し、実行中のオブジェクトを調べ、変更するためのツール群です。

```text
          Qt Cling
       Qt/C++ GUI REPL
             │
             ▼
           icpp
    practical REPL frontend


   Running Qt application
             │
             ▼
      ObjectSelector
    select QObject/QWidget
             │
             ▼
      PropertyEditor
    inspect/edit QObject
```

4つのツールは組み合わせて利用できます。
ObjectSelector と PropertyEditor は独立したライブラリーとしても利用できます。


### Qt Cling (`cling`)

Cling から Qt を対話的に利用するための環境です。

Qt の初期化とイベント処理を Cling に組み合わせることで、
端末から `QObject` や `QWidget` を生成し、その場で操作できます。

```cpp
QPushButton *button = new QPushButton("Hello");
button->resize(200, 80);
button->show();
```

通常の Qt/C++ コードを、コンパイルとリンクを繰り返さずに試すことを目的としています。


### icpp (`icpp`)

Cling / Qt Cling を実用的に使うための REPL ラッパーです。

icpp 自体は C++ を解釈せず、cling または qtcling をサブプロセスとして起動します。

主な機能:

- 複数行の C++ コードを編集バッファとして扱う
- 外部エディターで関数やクラスを編集する
- 編集したコードを最初から再実行する
- 複数ファイルを登録してまとめて再実行する
- クリップボード経由でコードを出し入れする
- 試したコードをファイルへ保存する
- Qt 型の値を簡単に表示する
- Qt の moc / uic / rcc を利用する
- MCP からコード評価、状態取得、リセットを行う

Cling をサブプロセスとして分離することで、
対話実行中の C++ コードによって Cling がクラッシュしても、
icpp 自体を生き残らせて作業を継続できるようにしています。


### ObjectSelector (`objectselector`)

Qt アプリケーション内の `QObject` / `QWidget` を選択するための
Qt Widgets ライブラリーです。

- 画面上の `QWidget` を直接指して選択
- オブジェクトツリーから選択
- 選択対象をハイライト表示

ObjectSelector は「選択」だけを担当します。
選択したオブジェクトの表示や編集は行いません。


### PropertyEditor (`propertyeditor`)

`QObject` / `QMetaObject` / `QMetaProperty` を利用して、
実行中の Qt オブジェクトのプロパティーを表示・編集するための
Qt Widgets ライブラリーです。

`QMetaProperty` と dynamic property を表示し、
型に応じたエディターで値を変更できます。

代表的な対応型:

- `bool`
- `int`
- `double`
- `QString`
- `QStringList`
- enum / flags
- `QPoint`
- `QSize`
- `QRect`
- `QColor`
- `QByteArray`
- dynamic property

ObjectSelector と組み合わせるほか、
任意の `QObject` を直接渡して単独で利用することもできます。


## Featured: App Generation Workflow

`app-generation-workflow` は、GUI アプリ開発を主眼とした、
ドキュメント駆動の AI 開発ハーネスです。
要求整理、仕様化、実装、テスト、レビューをつなぐ手順とテンプレートを提供します。

作りたいものを文章にし、利用シナリオ・画面と操作・業務ルールを整理して、
AI への実装指示へつなげます。生成後も仕様を基準に動作を確認し、
仕様と実装の対応を文書に残します。

### 要求から実装・確認へ

```text
要求のメモ (.txt)
       │ 要求定義レビュー
       ▼
仕様化前ノート (source_memo.md)
       │ 目的・操作・ルール・対象外・未決事項を整理
       ▼
三点セット
  ├─ UC仕様書             利用シナリオと期待する結果
  ├─ UI仕様書             画面・入力・表示・操作
  └─ ビジネスレイヤー仕様書 データ・検証・計算・保存
       │ レビュー後に実装方式を指定
       ▼
実装プロンプト → アプリ実装
       │ ビルド・テスト・手動確認・実装結果レビュー
       ▼
仕様とコードの対応を記録 → アプリ設計書
```

AI は、文書の草案作成、不足や矛盾の洗い出し、実装、テスト作成、照合を担当します。
人は、何を作るか、どの仕様を採用するか、生成結果を受け入れてよいかを判断します。
各工程のレビューでは、次へ進める条件と、修正が必要な場合の戻り先を確認します。

### 中心となる三点セット

仕様化前ノートに集めた材料を、次の3つの視点へ分けます。

| 文書 | ファイル名 | 記述すること |
|---|---|---|
| UC仕様書 | `01_usecase_spec.md` | 利用者の目的、操作の流れ、成功条件、例外時の振る舞い |
| UI仕様書 | `02_ui_spec.md` | 画面構成、入力と表示、操作できる条件、エラー表示 |
| ビジネスレイヤー仕様書 | `03_business_spec.md` | UIに依存しないデータ、検証、計算、保存、状態変更 |

例えば「ファイルを開く」なら、UC仕様書で利用の流れと失敗時の振る舞い、
UI仕様書で開く操作とエラー表示、ビジネスレイヤー仕様書で読み込みとデータの検証を定めます。
同じ機能を3つの視点で照合し、画面と内部処理の食い違いを見つけやすくします。

実装言語やフレームワークは、この整理を踏まえて実装プロンプトで指定します。
Qt 以外のプロジェクトにも利用できます。

### 生成後の確認と修正

主な確認・修正の手順:

- テストの期待値を仕様から導き、生成コードの出力と照合する
- 自動テストに加え、画面操作など必要な手動確認を行う
- 仕様判断を変える場合は、仕様化前ノート → 三点セット → 実装プロンプト → コードの順に反映する
- 仕様が正しく、転記漏れや実装不良だけがある場合は、根拠となる仕様を確認して該当工程へ戻す
- 上流の変更が影響する成果物を再確認し、確認結果と未確認事項を記録する
- 実装結果レビュー後、重大な不整合がないことを確認して、仕様とコードの対応やアプリ全体の設計を文書化する

作成した文書を次の変更にも使うことで、機能追加や意図違いの修正を、
要求・仕様・実装のつながりを確認しながら進められます。

### 始め方

[利用ガイド](app-generation-workflow/app-generation-workflow/USER_GUIDE.md) を読み、
リポジトリー内の `app-generation-workflow/app-generation-workflow/` を、
作業場所の `app-generation-workflow/` としてコピーします。
要求を書いた `.txt` ファイル（ここでは `requirement.txt`）を用意し、
AI に次のように依頼して始められます。

```text
app-generation-workflow/three_key_documents_workflow.md を読んでください。
requirement.txt の要求定義レビューをしてください。
不足や未決事項があれば、仕様化前ノートの作成前に示してください。
```

対象アプリ固有の仕様書・実装プロンプト・ソースコードは、
テンプレートと分けて対象アプリ用ディレクトリーへ置きます。
構成と詳しい手順は [ワークフローのREADME](app-generation-workflow/app-generation-workflow/README.md) と
[利用ガイド](app-generation-workflow/app-generation-workflow/USER_GUIDE.md) を参照してください。


## Featured: Work Templates

`work-templates` は、作業の現在地、次にやること、判断した理由を、
3つの Markdown に分けて残すための文書テンプレートです。
人への引き継ぎ、別の PC への移行、新しい AI セッションでの再開に利用できます。

「どこまで終わったか」「何が未確認か」「なぜこの方法を選んだか」を
プロジェクトと一緒に保存し、次の作業で読み直せるようにします。
アプリ生成ワークフローと組み合わせる場合は、仕様書やコードの参照先、
レビューの進み具合、次に確認する工程を残す役割を担います。

### 3つのファイルで作業を引き継ぐ

```text
作業・確認・判断
       │ 区切りで記録を更新
       ▼
プロジェクト
  ├─ NEXT.md       現在の状態と次の一手
  ├─ DECISIONS.md  判断・理由・継続する制約
  └─ backlog.md    後で取り組む課題と保留事項
       │ 必要な変更とともに Git で共有
       ▼
次の担当者・PC・AI セッション
       │ 記録を読み、実際のファイルや Git 状態と照合
       ▼
続きの作業を開始
```

| ファイル | 保存する情報 | 記入例 |
|---|---|---|
| `NEXT.md` | 現在の状態、次の1〜2作業、触るファイル、再開時の注意 | ファイル読み込みは実装済み。次は破損ファイルを開いた場合の動作を確認する |
| `DECISIONS.md` | 採用・不採用の判断と理由、未決の論点、継続する仕様や制約 | 読み込み失敗時は現在のデータを保持する。編集中の作業を失わないため |
| `backlog.md` | 直近では扱わない課題、問題点、保留理由と再検討条件 | 大容量ファイルの読み込み速度を、通常の読み込み動作を確認した後に検討する |

この例では、次の担当者は失敗時の動作確認から始められます。
期待する振る舞いの理由と、今回は扱わない課題も別々に確認できます。
詳しい検証ログや完了履歴は、報告書や Git 履歴などに残して参照します。

### 共通ルールと日々の使い方

`AGENTS.next.md` を各ホストの `~/AGENTS.next.md` に配置し、
複数のプロジェクトで同じ読み込み・更新の手順を使います。
各プロジェクトの `NEXT.md` 冒頭に、この共通ルールを先に読む指示があります。

作業の区切りでは、AI に次のように依頼します。

```text
NEXT.md を現在の作業状態に合わせて更新してください。
```

共通ルールでは、Git の差分、直近のコミット、関連文書を確認し、
3つのファイルを必要に応じて更新するよう指示しています。
次回の作業は `NEXT.md`、継続する判断は `DECISIONS.md`、後で扱う課題は `backlog.md` に整理します。

再開するときは、次のように依頼します。

```text
NEXT.md を読んでください。
```

共通ルールに従って残りの2ファイルも読み、記録を手掛かりに作業状態を確認します。
会話や AI の記憶だけに頼らず、保存した文書を読み直して引き継ぐ仕組みです。
作業の区切りで未記録の変更がある場合は更新を促す案内を行い、更新は依頼に基づいて実施します。

### 始め方と別のPCでの再開

1. `work-templates/AGENTS.next.md` を `~/AGENTS.next.md` に配置します。
2. 作業管理ファイルがない新規プロジェクトでは、`work-templates/` 内の `NEXT.md`、`DECISIONS.md`、`backlog.md` をコピーします。既にある場合は上書きせず、その内容を引き継ぎます。
3. コピーしたテンプレートの説明用の項目を実際の作業内容へ置き換えます。
4. 3ファイルを Git で管理し、再開に必要なソースや文書の変更も一緒に取得できるようにします。

別の PC では、同じ共通ルールを配置し、必要な変更を取得してから記録を読み直します。
未コミット変更がある場合は、その存在を記録するとともに、変更本体もコミットやパッチで渡します。
必要なブランチ、基準コミット、ツールの版、環境条件も再開時の手掛かりになります。

コピー手順と記入例は [work-templatesのREADME](work-templates/README.md) と
[利用ガイド](work-templates/USER_GUIDE.md) を参照してください。


## All Tools

### Interactive C++ / Qt

| Directory | Tool | Description |
|---|---|---|
| [cling](cling/README.txt) | Qt Cling / qtcling | Cling 上で Qt/C++ を対話的に実行します |
| [icpp](icpp/README.md) | icpp | Cling / Qt Cling を実用的に扱う REPL ラッパーです |


### Qt Object Inspection

| Directory | Tool | Description |
|---|---|---|
| [objectselector](objectselector/USER_GUIDE.md) | ObjectSelector | `QObject` / `QWidget` を画面またはオブジェクトツリーから選択します |
| [propertyeditor](propertyeditor/README.md) | PropertyEditor | `QObject` のプロパティーを表示・編集します |


### Qt Widgets Layouts

| Directory | Tool | Description |
|---|---|---|
| [anker-layout](anker-layout/README.md) | Anker Layout | Motif の `XmForm` に近い attachment レイアウトを Qt Widgets で利用するための `QLayout` 実装です。フォーム端、他のウィジェット、割合位置を基準に配置します |


### QML Tools

| Directory | Tool | Description |
|---|---|---|
| [qmlprop](qmlprop/README.md) | qmlprop | QML 型のプロパティー、メソッド、列挙子、継承関係などを調査します |
| [qmlprofileranalyzer](qmlprofileranalyzer/USER_GUIDE.md) | qmlprofileranalyzer | QML Profiler の `.qtd` / `.qzt` トレースを解析するコマンドラインツールです。概要・ホットスポット・イベントの親子関係の確認、2 つのトレースの比較、形式の相互変換に対応します |


### Development Workflows, Instructions and Templates

| Directory | Tool | Description |
|---|---|---|
| [app-generation-workflow](app-generation-workflow/app-generation-workflow/README.md) | App generation workflow | GUI アプリ開発を主眼とした、ドキュメント駆動の AI 開発ハーネスです。要求整理、仕様化、実装、テスト、レビューを文書でつなぎます |
| [ai-cmake](ai-cmake/USER_GUIDE.md) | AI CMake instructions | 生成 AI が Qt 6 の `CMakeLists.txt` を生成・修正するための指示と検証環境です |
| [work-templates](work-templates/README.md) | Work context templates | 作業状態・長期課題・設計判断を Markdown で分離して管理し、人、ホスト、AIセッションが変わっても作業を再開できるようにします |


これらは、開発の進め方（app-generation-workflow）、ビルド定義の作り方（ai-cmake）、
作業の継続方法（work-templates）をそれぞれ支援します。
app-generation-workflow と work-templates は、Qt 以外のプロジェクトにも利用できます。

### Documents, Images and Diagrams

| Directory | Tool | Description |
|---|---|---|
| [markdown](markdown/USER_GUIDE.md) | genpdf | Markdown から文書やスライドの PDF を生成します |
| [cutter](cutter/USER_GUIDE.md) | cutter | 数値指定で画像を切り抜きます。背景の透明化にも対応します |
| [whiteboard](whiteboard/whiteboard-app/USER_GUIDE.md) | WhiteboardApp | 説明図を作成・編集する Whiteboard。複数ページ、PNG/SVG 出力、MCP に対応します |


### General-purpose Command-line Utilities

| Directory | Tool | Description |
|---|---|---|
| [dateorder](dateorder/USER_GUIDE.md) | younger / older | ファイルが更新日時順に並んでいるかを判定します |
| [dtree](dtree/USER_GUIDE.md) | dtree | ディレクトリー構造を枝線付きのツリーとして表示します |
| [now](now/USER_GUIDE.md) | now | 現在日時をさまざまな形式で表示します |
| [tl](tl/USER_GUIDE.md) | tl | 日本語と英語を自動判定して相互翻訳します |


### Presentation and Seminar Tools

| Directory | Tool | Description |
|---|---|---|
| [LaserPointer](LaserPointer/README.md) | LaserPointer | 画面上にレーザー光点を表示する説明用ツールです。ドラッグ移動、マウスポインター追従、点滅、軌跡に対応します |
| [seminartimer](seminartimer/USER_GUIDE.md) | SeminarTimer | セミナー、演習、休憩などの残り時間を表示するタイマーです |

説明図の作成や画面共有には、[Documents, Images and Diagrams](#documents-images-and-diagrams) の WhiteboardApp も利用できます。


### Desktop Accessories

| Directory | Tool | Description |
|---|---|---|
| [qeyes](qeyes/README.md) | Qeyes | classic Xeyes を Qt Widgets で再実装したデスクトップアクセサリーです |
| [vu-stereo](vu-stereo/README.md) | VuStereo | macOS のシステム再生音をアナログ風 VU メーターで表示します |


## Documentation

各ツールの詳しい使い方、ビルド方法、動作環境については、
それぞれのディレクトリーにある `README.md`、`README.txt`、
`USER_GUIDE.md` などを参照してください。

アプリ開発ワークフローの入口は、[README](app-generation-workflow/app-generation-workflow/README.md) と
[利用ガイド](app-generation-workflow/app-generation-workflow/USER_GUIDE.md) です。

ツールごとに対象 OS、Qt のバージョン、外部ライブラリーやサービスへの依存が異なります。

リポジトリー直下の `NEXT.md`、`DECISIONS.md`、`backlog.md` は、maintainer が
`qt-tools` の開発で実際に使用している作業コンテキストです。現在の作業状態、設計判断、
残課題を複数ホストや AI セッション間で共有するために Git 管理しています。
clone した利用者がそのまま自分の作業コンテキストとして使うためのテンプレートではありません。
再利用可能なテンプレートは [work-templates/](work-templates/README.md) を参照してください。


## Platforms

ツールによって対応環境は異なります。

主に次の環境を対象としています。

- macOS
- Linux
- Windows
- Qt 6

コマンド系は、genpdf など一部を除き、macOS と Linux を対象としています。

一部のツールには個別の制約があります。

- VuStereo は macOS 専用です。
- `tl` は Google Cloud Translation の認証設定を必要とします。
- Qt Cling / icpp は Cling および対応する Qt 環境を必要とします。

詳細は各ツールのドキュメントを参照してください。


## About This Repository

ここにあるツールの多くは、実際の Qt/C++ 開発やセミナー、
資料作成などで必要になったものを作り、使用しながら改良してきたものです。

大きな統合環境を作ることよりも、
それぞれのツールが単独でも利用でき、必要に応じて組み合わせられることを
重視しています。
