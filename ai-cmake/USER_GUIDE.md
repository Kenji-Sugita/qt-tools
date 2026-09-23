---
genpdf:
  format: book
  title: 生成 AI 向け CMake
  subtitle: 指示ファイル 利用ガイド
  author: (株) SRA
  date: 2026-05-07
  font_size: 11pt
  page_numbers: true
---

# 生成 AI 向け CMake 指示ファイル 利用ガイド

このガイドは、`agents/AGENTS.md` と `agents/AGENTS.cmake.md` を使って、生成 AI が
Qt 6 用の `CMakeLists.txt` を生成・修正するときの出力方針をそろえるための利用手順
を説明します。

Codex での利用を基本例として説明しますが、`AGENTS.cmake.md` に書かれている CMake
生成方針は、他の生成 AI でも利用できます。Codex 以外で使う場合は、対象の AI に
`AGENTS.cmake.md` の内容を読ませたうえで、その指示に従って `CMakeLists.txt` を生
成・修正するよう依頼してください。

## 1. このガイドの目的

このリポジトリには、生成 AI が作成・更新する `CMakeLists.txt` の方針をそろえるた
めの指示ファイルが入っています。

このガイドでは、次のことを説明します。

- 各ファイルの役割
- 基本的な配置方法
- 生成 AI への依頼方法
- `AGENTS.cmake.md` に含まれる 33 パターン
- 生成された `CMakeLists.txt` の確認方法
- よくあるトラブルへの対応

## 2. なぜ指示ファイルを使うのか

毎回生成 AI に `CMakeLists.txt` を生成させるだけでも、単発の作業では十分な場合が
あります。

ただし、Qt / CMake のように正解が複数あり、古い書き方と新しい書き方が混在しやす
い領域では、毎回の依頼だけに任せると出力がぶれやすくなります。

また、コード追加に合わせて生成 AI が `CMakeLists.txt` を更新できる場合でも、その
更新方法が毎回同じ方針になるとは限りません。

指示ファイルを用意しておくと、生成 AI の判断を自分の標準方針に寄せられます。

### 2.1 毎回 AI に依頼する場合との違い

毎回 AI に依頼する方法では、その場の条件を柔軟に伝えられます。一方で、
`qt_standard_project_setup()` を使うか、QML を `qt_add_qml_module()` で登録するか、
`.qrc` を使うかなどの判断が依頼ごとに揺れる可能性があります。

指示ファイルを使う方法では、あらかじめ標準方針をファイルに書いておきます。そのた
め、同じルールを複数プロジェクトで繰り返し使えます。

### 2.2 指示ファイルを用意する利点

- 出力のぶれを減らせる。
- Qt 6 向けの現代的な CMake 方針を固定できる。
- 毎回長いプロンプトを書かずに済む。
- 既存 `CMakeLists.txt` の修正やレビューにも同じ基準を使える。
- コード追加に伴う `CMakeLists.txt` 更新でも、場当たり的な判断を減らせる。
- `qmake` の `.pro` ファイルから Qt 6 / CMake へ移行するときのたたき台を作りやす
  い。
- 複数リポジトリで同じ CMake 方針を共有できる。

### 2.3 指示ファイルの注意点

- `AGENTS.cmake.md` は必要に応じて更新する。
- 特殊なプロジェクトでは、依頼文やリポジトリ側の `AGENTS.md` で例外を明示する。
- 指示が強すぎると、プロジェクト固有の事情に合わない場合がある。
- 生成結果は必ずビルドして確認する。

### 2.4 使い分けの目安

単発の実験や小さなサンプルでは、毎回生成 AI に直接依頼するだけでも十分です。

一方、次のような場合は指示ファイルを使う方が向いています。

- Qt / CMake プロジェクトを何度も扱う。
- 複数リポジトリで同じ CMake 方針を使いたい。
- 古い Qt CMake の書き方を避けたい。
- QML の登録方法を安定させたい。
- 生成結果のレビュー基準をそろえたい。
- コード追加時の `CMakeLists.txt` 更新方針をそろえたい。
- `qmake` から CMake への移行を進めたい。

### 2.5 おすすめの運用

基本は指示ファイルを使い、例外だけを依頼文で補足します。

Codex で `AGENTS.md` を置いたリポジトリを開いている場合は、次のような短い依頼でも
`AGENTS.cmake.md` の方針に沿った `CMakeLists.txt` を作成しやすくなります。

```bash
codex "CMakeLists.txt を作成してください"
```

例:

```text
AGENTS.md と ~/AGENTS.cmake.md に従って CMakeLists.txt を作成してください。
このプロジェクトは Qt 6.2 対応が必要です。
```

```text
AGENTS.md と ~/AGENTS.cmake.md に従ってください。
ただし既存の qrc:/... の読み込み URL は変更しないでください。
```

```text
コード追加に伴って CMakeLists.txt を更新してください。
更新時も AGENTS.md と ~/AGENTS.cmake.md の方針に従ってください。
```

この運用にすると、標準方針は指示ファイルに任せつつ、プロジェクト固有の事情だけを
生成 AI に伝えられます。

## 3. 全体像

### 3.1 このリポジトリに含まれるファイル

主なファイルは次の 2 つです。

```text
agents/
├── AGENTS.md
└── AGENTS.cmake.md
```

`AGENTS.md` は、作業対象リポジトリに置く基本指示です。

`AGENTS.cmake.md` は、Qt 6 用の `CMakeLists.txt` を生成・修正するための詳しい指示
です。

### 3.2 AGENTS.md と AGENTS.cmake.md の関係

基本運用では、`AGENTS.md` を各リポジトリに置き、`AGENTS.cmake.md` をホームディレ
クトリに置きます。

`AGENTS.md` には、`CMakeLists.txt` を作成・修正・更新する前に
`~/AGENTS.cmake.md` を読むように書いてあります。

これにより、複数のリポジトリで同じ CMake 生成ルールを共有できます。

### 3.3 生成 AI が CMake 指示を読む流れ

Codex で使う場合の基本的な流れは次の通りです。

1. Codex が作業対象リポジトリの `AGENTS.md` を読む。
2. `CMakeLists.txt` の作成・修正・更新が必要な場合、`~/AGENTS.cmake.md` を読む。
3. `AGENTS.cmake.md` の方針とパターンに従って `CMakeLists.txt` を生成・修正する。

Codex 以外の生成 AI で使う場合は、`AGENTS.md` や `AGENTS.cmake.md` が自動的に読ま
れるとは限りません。その場合は、依頼時に対象ファイルを添付する、本文を貼り付ける、
または「この指示ファイルを読んで従ってください」と明示してください。

## 4. 基本の配置

### 4.1 AGENTS.md をリポジトリールートに置く

作業対象リポジトリのルートに `AGENTS.md` を置きます。

```text
your-project/
├── AGENTS.md
├── CMakeLists.txt
└── src/
```

### 4.2 AGENTS.cmake.md をホームディレクトリに置く

`AGENTS.cmake.md` は、ホームディレクトリに `~/AGENTS.cmake.md` として置きます。

```text
~/
└── AGENTS.cmake.md
```

### 4.3 基本配置のディレクトリ例

この配置では、作業対象リポジトリと共通 CMake 指示が分かれます。

```text
~/
└── AGENTS.cmake.md

your-project/
├── AGENTS.md
├── CMakeLists.txt
├── src/
└── tests/
```

## 5. セットアップ手順

### 5.1 AGENTS.md をコピーする

このリポジトリの `agents/AGENTS.md` を、作業対象リポジトリのルートにコピーします。

例:

```text
agents/AGENTS.md -> your-project/AGENTS.md
```

### 5.2 AGENTS.cmake.md を ~/AGENTS.cmake.md として配置する

このリポジトリの `agents/AGENTS.cmake.md` を、ホームディレクトリに配置します。

例:

```text
agents/AGENTS.cmake.md -> ~/AGENTS.cmake.md
```

### 5.3 AGENTS.md の参照先を確認する

`AGENTS.md` に、次のような指示があることを確認します。

```markdown
`CMakeLists.txt` を新規作成または更新するときは、ソースコードだけから自由に判断せ
ず、必ず `~/AGENTS.cmake.md` の方針と該当パターンに従うこと。
コード追加に伴って CMakeLists.txt を更新する場合も同じルールを適用すること。
```

別の場所に `AGENTS.cmake.md` を置く場合は、この参照先を書き換えてください。

### 5.4 生成 AI に認識させる

Codex で使う場合は、作業対象リポジトリを Codex で開き、通常どおり
`CMakeLists.txt` の作成・修正・更新を依頼します。

他の生成 AI で使う場合は、`AGENTS.cmake.md` の内容を読ませたうえで、同じ方針に従
うよう依頼します。

依頼時に、必要であれば次のように明示します。

```text
AGENTS.md と ~/AGENTS.cmake.md の指示に従ってください。
```

## 6. 基本的な使い方

### 6.1 CMakeLists.txt を新規作成する

新しい Qt プロジェクトに `CMakeLists.txt` がない場合は、プロジェクトの種類を伝え
て依頼します。

例:

```text
この Qt Widgets アプリケーション用の CMakeLists.txt を作成してください。
```

### 6.2 既存の CMakeLists.txt を修正する

既存の `CMakeLists.txt` を Qt 6 向けに整えたい場合は、既存の挙動を維持することを
明示します。

例:

```text
既存の動作を維持したまま、この CMakeLists.txt を Qt 6 の現代的な CMake に修正してください。
```

### 6.3 Qt Widgets アプリ用に依頼する

Widgets アプリでは、`QApplication` や `QMainWindow` を使っていることを伝えると、
生成 AI が `Widgets` パターンを選びやすくなります。

例:

```text
QApplication と QMainWindow を使う Widgets アプリです。CMakeLists.txt を作成してください。
```

### 6.4 Qt Quick / QML アプリ用に依頼する

QML アプリでは、QML の読み込み方法を伝えることが重要です。

`loadFromModule()` を使う場合:

```text
Qt Quick アプリです。main.cpp は loadFromModule() を使う前提で、qt_add_qml_module() の構成にしてください。
```

`qrc:/...` を使う場合:

```text
既存の main.cpp は qrc:/qt/qml/app/Main.qml を読み込みます。この URL を壊さない CMakeLists.txt にしてください。
```

### 6.5 テスト構成を追加する

Qt Test を使う場合は、テスト対象をアプリ本体から分けることを依頼します。

例:

```text
Qt Test を使うテストターゲットを追加してください。テスト対象のロジックはライブラリとして分離してください。
```

### 6.6 コード追加に合わせて CMakeLists.txt を更新する

コードを追加したあとに `CMakeLists.txt` を更新する場合も、生成 AI に自由に判断さ
せるのではなく、指示ファイルの方針に従うように依頼します。

例:

```text
追加したソースファイルと使用している Qt クラスを確認して、CMakeLists.txt を更新してください。
更新時も AGENTS.md と ~/AGENTS.cmake.md の方針に従ってください。
```

### 6.7 qmake から CMake へ移行する

既存の `.pro` ファイルから CMake へ移行する場合は、`.pro` の設定を機械的に写すだ
けでなく、Qt 6 向けの現代的な CMake に寄せることを依頼します。

例:

```text
この .pro ファイルとソース構成を確認して、Qt 6 用の CMakeLists.txt を作成してください。
既存のターゲット名、ソースファイル、Qt モジュール構成を保ちつつ、AGENTS.cmake.md の方針に従ってください。
```

## 7. 生成 AI への依頼例

### 7.1 新規 CMakeLists.txt の作成依頼

Codex で `AGENTS.md` を配置済みのリポジトリを開いている場合:

```bash
codex "CMakeLists.txt を作成してください"
```

この短い依頼でも、`AGENTS.md` から `~/AGENTS.cmake.md` を参照するため、共通方針に
沿った `CMakeLists.txt` を作成しやすくなります。

プロジェクト固有の条件を明示したい場合:

```text
このディレクトリのソースを確認して、Qt 6 用の CMakeLists.txt を新規作成してください。
AGENTS.md と ~/AGENTS.cmake.md の指示に従ってください。
```

### 7.2 既存 CMakeLists.txt の現代化依頼

```text
既存の CMakeLists.txt を Qt 6 の現代的な CMake に修正してください。
既存のターゲット名と実行時の挙動は維持してください。
```

### 7.3 QML リソース構成の修正依頼

```text
QML の登録方法を見直してください。
main.cpp の読み込み方法に合わせて、qt_add_qml_module() または qt_add_resources() を使う構成にしてください。
```

### 7.4 Qt Test 追加の依頼

```text
Qt Test のテストターゲットを追加してください。
CTest から実行できるように add_test() も設定してください。
```

### 7.5 ビルドエラー修正の依頼

```text
このビルドエラーを直してください。
CMakeLists.txt の変更は原因に関係する最小限にしてください。
```

### 7.6 コード追加に伴う CMakeLists.txt 更新の依頼

```text
追加されたソースファイルと使用している Qt モジュールを確認して、CMakeLists.txt を更新してください。
更新時も AGENTS.md と ~/AGENTS.cmake.md の方針に従ってください。
```

### 7.7 qmake から CMake への移行依頼

```text
既存の .pro ファイルを確認して、Qt 6 用の CMakeLists.txt を作成してください。
既存の構成を尊重しつつ、AGENTS.cmake.md の方針に従って現代的な CMake にしてください。
```

### 7.8 生成結果レビューの依頼

```text
生成された CMakeLists.txt をレビューしてください。
Qt モジュールの指定、QML の登録方法、ターゲット設定に問題がないか確認してください。
```

## 8. AGENTS.cmake.md に含まれる 33 パターン

### 8.1 アプリケーション系パターン

- Qt Widgets アプリケーション
- Qt Quick / QML アプリケーション
- Qt コンソールアプリケーション
- 複数実行ファイルを持つプロジェクト

### 8.2 ライブラリ系パターン

- Qt Core ライブラリ
- Qt Widgets ライブラリ
- アプリケーション + 共通ライブラリ

### 8.3 テスト系パターン

- Qt Test を使うテストターゲット
- アプリケーション + テスト構成

### 8.4 サブディレクトリ構成

- ルート `CMakeLists.txt`
- `src/CMakeLists.txt`
- `tests/CMakeLists.txt`
- `add_subdirectory()` を使う構成

### 8.5 Qt モジュール別パターン

- Qt Designer の `.ui` ファイルを使う Widgets アプリ
- リソースファイルを使う Widgets アプリ
- Qt Network を使うアプリケーション
- Qt SQL を使うアプリケーション
- Qt Concurrent を使うアプリケーション
- OpenGL / Qt OpenGL を使うアプリケーション
- 翻訳ファイルを扱うプロジェクト

### 8.6 配布・互換性パターン

- `install()` を含む構成
- macOS / Windows GUI アプリ用プロパティ
- Qt 6.3 未満に対応する構成
- バージョンなし Qt CMake コマンドが無効な環境

### 8.7 QML 応用パターン

- Qt Quick Controls アプリケーション
- 複数 QML ファイルを持つアプリケーション
- QML リソースを含むアプリケーション
- C++ バックエンドを QML に公開するアプリケーション
- QML singleton を使うアプリケーション
- 既存 `qrc:/...` 読み込みを維持する QML アプリケーション
- QML モジュールをライブラリ化する構成
- 複数 QML モジュールを持つ構成
- C++ 型を含む QML モジュール
- QML アプリケーション + テスト構成
- QML アプリケーションの `install()` 構成
- Qt 6.3 未満向け QML 構成

## 9. 生成される CMakeLists.txt の基本方針

### 9.1 Qt 6 の現代的な CMake を使う

`AGENTS.cmake.md` は、Qt 6 向けの現代的な CMake を前提にしています。

古い Qt 5 風の書き方や、グローバル設定に依存する書き方は原則として避けます。

### 9.2 qt_standard_project_setup() を使う

Qt 6.3 以降を前提にできる場合は、`find_package(Qt6 ...)` の直後に
`qt_standard_project_setup()` を呼び出します。

### 9.3 qt_add_executable() / qt_add_library() を使う

Qt アプリケーションでは `qt_add_executable()` を優先します。

Qt ライブラリでは、必要に応じて `qt_add_library()` を使います。

### 9.4 Qt インポートターゲットをリンクする

Qt モジュールは、`Qt6::Widgets` や `Qt6::Quick` のようなインポートターゲットでリ
ンクします。

### 9.5 不要なグローバル設定を避ける

新規作成時は、次のようなグローバル設定を避けます。

- `include_directories()`
- `link_libraries()`
- 不必要な `file(GLOB ...)`
- 不必要な手動 `CMAKE_AUTOMOC` / `CMAKE_AUTOUIC`

## 10. 配置パターンの応用

### 10.1 AGENTS.cmake.md をホームに置く

基本の置き方です。

複数リポジトリで共通の CMake 方針を使う場合に向いています。

```text
~/
└── AGENTS.cmake.md

your-project/
└── AGENTS.md
```

### 10.2 AGENTS.cmake.md をリポジトリ内に置く

プロジェクトごとに CMake 方針を変えたい場合は、`AGENTS.cmake.md` をリポジトリ内に
置けます。

```text
your-project/
├── AGENTS.md
├── AGENTS.cmake.md
└── CMakeLists.txt
```

この場合は、`AGENTS.md` の参照先を `./AGENTS.cmake.md` に変更してください。

### 10.3 サブディレクトリごとに AGENTS.md を置く

大きなリポジトリで Qt プロジェクトが一部だけにある場合は、そのサブディレクトリに
専用の `AGENTS.md` を置けます。

```text
repository/
├── AGENTS.md
└── qt-app/
    ├── AGENTS.md
    └── CMakeLists.txt
```

### 10.4 AGENTS.md にすべての指示をまとめる

小さなプロジェクトでは、`AGENTS.md` に CMake 用の指示を直接まとめても構いません。

ただし、複数プロジェクトで使い回す場合は、`AGENTS.cmake.md` を分けたほうが更新し
やすくなります。

### 10.5 別名ファイルを参照する

別名の指示ファイルを使う場合は、`AGENTS.md` の参照文を実際のファイル名に合わせて
ください。

例:

```markdown
`CMakeLists.txt` を新規作成または更新するときは、必ず `./docs/CMakeRules.md` を読
んで従うこと。
```

## 11. 生成結果の確認方法

### 11.1 CMake の構文を確認する

生成された `CMakeLists.txt` に、少なくとも次があることを確認します。

- `cmake_minimum_required()`
- `project()`
- `find_package(Qt6 REQUIRED COMPONENTS ...)`
- ターゲット定義
- `target_link_libraries()`

### 11.2 必要な Qt モジュールを確認する

使っている Qt クラスに対応するモジュールが `find_package()` と
`target_link_libraries()` に入っているか確認します。

例:

- QWidget 系: `Widgets`
- QML / Qt Quick: `Quick`
- ネットワーク: `Network`
- SQL: `Sql`
- Qt Test: `Test`

### 11.3 QML 読み込み方法と CMake の対応を確認する

`main.cpp` の QML 読み込み方法と CMake の登録方法が一致しているか確認します。

- `loadFromModule()` を使う場合: `qt_add_qml_module()`
- `qrc:/...` を使う場合: `qt_add_resources()`

### 11.4 GUI / Console / Test のターゲット設定を確認する

GUI アプリには、必要に応じて次が設定されます。

```cmake
set_target_properties(MyApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
```

コンソールアプリやテストターゲットには、通常これらを設定しません。

### 11.5 ビルドして確認する

最後に CMake の configure と build を実行して確認します。

例:

```text
cmake -S . -B build
cmake --build build
```

Qt Creator を使う場合は、プロジェクトを開いて Configure と Build が通るか確認しま
す。

## 12. よくあるトラブル

### 12.1 指示ファイルが読まれていない

`AGENTS.md` の参照先が正しいか確認してください。

基本配置では、参照先は `~/AGENTS.cmake.md` です。

### 12.2 qt_standard_project_setup() が見つからない

Qt 6.3 未満を使っている可能性があります。

その場合は、`AGENTS.cmake.md` の「Qt 6.3 未満に対応する構成」を使うように生成 AI
へ依頼してください。

### 12.3 QML ファイルが実行時に見つからない

`main.cpp` の読み込み URL と、CMake の QML 登録方法が一致しているか確認してくださ
い。

`loadFromModule()` と `qrc:/...` では、使う CMake コマンドが異なります。

### 12.4 必要な Qt モジュールがリンクされていない

使っている Qt クラスに対応するモジュールが不足している可能性があります。

生成 AI には、エラー全文と使っている Qt クラスを渡して修正を依頼してください。

### 12.5 Qt 6.3 未満で失敗する

`qt_standard_project_setup()` は Qt 6.3 で導入された関数です。

Qt 6.2 などを使う場合は、Qt 6.3 未満対応のパターンを使います。

### 12.6 バージョンなし Qt CMake コマンドが使えない

環境によっては、`qt_add_executable()` などのバージョンなしコマンドが使えない場合
があります。

その場合は、`qt6_add_executable()` や `qt6_standard_project_setup()` を使うパター
ンを依頼してください。

## 13. 運用のコツ

### 13.1 プロジェクト固有ルールを AGENTS.md に書く

ターゲット名、ディレクトリ構成、対応 Qt バージョンなど、プロジェクト固有の制約は
作業対象リポジトリの `AGENTS.md` に書くと扱いやすくなります。

### 13.2 共通ルールを AGENTS.cmake.md にまとめる

Qt CMake の基本方針や生成パターンは、`AGENTS.cmake.md` にまとめます。

複数リポジトリで同じルールを使えるため、運用が安定します。

コード追加に伴って `CMakeLists.txt` を更新する場合も、この共通ルールに従わせると、
プロジェクト間で出力をそろえやすくなります。

### 13.3 例外条件を依頼文で明示する

Qt 6.2 を使う、既存の `.qrc` を維持したい、特定のターゲット名を変えたくないなど
の条件は、生成 AI への依頼文で明示してください。

### 13.4 生成後にレビューさせる

生成された `CMakeLists.txt` は、そのまま終わりにせず、生成 AI にレビューさせると
見落としを減らせます。

例:

```text
この CMakeLists.txt をレビューして、AGENTS.cmake.md の方針に合っているか確認してください。
```

### 13.5 複数リポジトリで使う場合の更新方針

`~/AGENTS.cmake.md` を更新すると、複数リポジトリでの生成 AI の判断に影響します。

大きく変更する場合は、必要に応じてプロジェクトごとの `AGENTS.md` に例外や補足を書
いてください。

## 14. 付録

### 14.1 AGENTS.md の最小例

```markdown
# AGENTS.md

## Change Policy

- 指示にない機能追加や仕様変更は、実装前に必ず提案し、許可を得ること。
- リファクタでは既存仕様の維持を優先し、必要最小限の変更にとどめること。

## 関連する追加指示

- `CMakeLists.txt` を新規作成または更新するときは、ソースコードだけから自由に判断
  せず、必ず `~/AGENTS.cmake.md` の方針と該当パターンに従うこと。
- コード追加に伴って `CMakeLists.txt` を更新する場合も同じルールを適用すること。
```

### 14.2 基本配置の例

```text
~/
└── AGENTS.cmake.md

your-project/
├── AGENTS.md
├── CMakeLists.txt
├── src/
└── tests/
```

### 14.3 生成 AI への依頼テンプレート

```text
このプロジェクトの CMakeLists.txt を作成してください。
AGENTS.md と ~/AGENTS.cmake.md の指示に従ってください。

条件:
- Qt 6 を使います。
- アプリケーション種別は <Widgets / Quick / Console> です。
- 既存のターゲット名は変更しないでください。
- 必要ならテストターゲットも追加してください。
- qmake から移行する場合は、既存の .pro ファイルの構成を尊重してください。
```

### 14.4 33 パターン一覧

1. Qt Widgets アプリケーション
2. Qt Quick / QML アプリケーション
3. Qt コンソールアプリケーション
4. Qt Core ライブラリ
5. Qt Widgets ライブラリ
6. アプリケーション + 共通ライブラリ
7. 複数実行ファイルを持つプロジェクト
8. Qt Test を使うテストターゲット
9. アプリケーション + テスト構成
10. サブディレクトリ構成
11. Qt Designer の `.ui` ファイルを使う Widgets アプリ
12. リソースファイルを使う Widgets アプリ
13. Qt Network を使うアプリケーション
14. Qt SQL を使うアプリケーション
15. Qt Concurrent を使うアプリケーション
16. OpenGL / Qt OpenGL を使うアプリケーション
17. 翻訳ファイルを扱うプロジェクト
18. `install()` を含む構成
19. macOS / Windows GUI アプリ用プロパティ
20. Qt 6.3 未満に対応する構成
21. バージョンなし Qt CMake コマンドが無効な環境
22. Qt Quick Controls アプリケーション
23. 複数 QML ファイルを持つアプリケーション
24. QML リソースを含むアプリケーション
25. C++ バックエンドを QML に公開するアプリケーション
26. QML singleton を使うアプリケーション
27. 既存 `qrc:/...` 読み込みを維持する QML アプリケーション
28. QML モジュールをライブラリ化する構成
29. 複数 QML モジュールを持つ構成
30. C++ 型を含む QML モジュール
31. QML アプリケーション + テスト構成
32. QML アプリケーションの `install()` 構成
33. Qt 6.3 未満向け QML 構成
