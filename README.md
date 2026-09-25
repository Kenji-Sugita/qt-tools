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


### Command-line Utilities

| Directory | Tool | Description |
|---|---|---|
| [dateorder](dateorder/USER_GUIDE.md) | younger / older | ファイルが更新日時順に並んでいるかを判定します |
| [dtree](dtree/USER_GUIDE.md) | dtree | ディレクトリー構造を枝線付きのツリーとして表示します |
| [now](now/USER_GUIDE.md) | now | 現在日時をさまざまな形式で表示します |
| [tl](tl/USER_GUIDE.md) | tl | 日本語と英語を自動判定して相互翻訳します |


### Desktop Tools and Experiments

| Directory | Tool | Description |
|---|---|---|
| [seminartimer](seminartimer/USER_GUIDE.md) | SeminarTimer | セミナー、演習、休憩などの残り時間を表示するタイマーです |
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
