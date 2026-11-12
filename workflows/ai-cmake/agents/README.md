# agents 利用ガイド

このディレクトリの Markdown ファイルは、Codex が `CMakeLists.txt` を生成・修正・
更新するときの出力方針をそろえる追加指示として使います。

基本の使い方は、次の配置です。

- `AGENTS.md` を作業対象リポジトリのルートに置く。
- `AGENTS.cmake.md` をホームディレクトリに `~/AGENTS.cmake.md` として置く。

この配置にすると、作業対象リポジトリの `AGENTS.md` から CMake 用の共通指示
`~/AGENTS.cmake.md` を参照できます。複数のリポジトリで同じ CMake 方針を使いたい
場合に向いています。

## ファイルの役割

### AGENTS.md

`AGENTS.md` は、作業対象リポジトリに置く Codex 向けの基本指示です。

主な役割:

- 指示にない機能追加や仕様変更を勝手に行わないようにする。
- リファクタ時に既存仕様の維持を優先させる。
- `CMakeLists.txt` を新規作成または更新するときに、`~/AGENTS.cmake.md` の方針と該
  当パターンに従うように指示する。
- コード追加に伴う `CMakeLists.txt` 更新でも、同じルールを適用させる。

基本配置:

```text
your-project/
├── AGENTS.md
├── CMakeLists.txt
└── src/
```

### AGENTS.cmake.md

`AGENTS.cmake.md` は、Qt 6 用の `CMakeLists.txt` を生成・修正するための詳しい
Codex 指示です。

主な役割:

- 現代的な Qt CMake の書き方を指定する。
- `find_package(Qt6 REQUIRED COMPONENTS ...)`、`qt_standard_project_setup()`、
  `qt_add_executable()`、Qt インポートターゲットの利用を指示する。
- 既存 `CMakeLists.txt` を修正するときの方針を指定する。
- 非推奨の CMake 書き方を避けさせる。
- よく使う `CMakeLists.txt` の生成パターンを Codex に提示する。

基本配置:

```text
~/
└── AGENTS.cmake.md
```

## AGENTS.cmake.md に含まれる 33 パターン

`AGENTS.cmake.md` には、次の `CMakeLists.txt` 生成パターンが入っています。

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

## 基本セットアップ

1. 作業対象リポジトリのルートに `AGENTS.md` を置きます。
2. ホームディレクトリに `AGENTS.cmake.md` を置きます。
3. Codex に `CMakeLists.txt` の作成・修正・更新を依頼します。

依頼例:

```text
この Qt Widgets プロジェクト用の CMakeLists.txt を作成してください。
```

```text
既存の CMakeLists.txt を Qt 6 の現代的な CMake に修正してください。
```

```text
この Qt Quick アプリを qt_add_qml_module() を使う構成にしてください。
```

## 補足: ほかの置き方

基本は `AGENTS.md` をリポジトリールート、`AGENTS.cmake.md` をホームに置く構成です。
ただし、運用に合わせて別の置き方もできます。

### リポジトリごとに CMake 指示を変える

プロジェクトごとに CMake 方針を変えたい場合は、`AGENTS.cmake.md` もリポジトリ内に
置けます。

```text
your-project/
├── AGENTS.md
├── AGENTS.cmake.md
└── CMakeLists.txt
```

この場合は、`AGENTS.md` の参照先を `~/AGENTS.cmake.md` ではなく
`./AGENTS.cmake.md` などに変更してください。

### サブディレクトリだけに適用する

大きなリポジトリで Qt プロジェクトが一部だけにある場合は、そのサブディレクトリに
専用の `AGENTS.md` を置けます。

```text
repository/
├── AGENTS.md
└── qt-app/
    ├── AGENTS.md
    ├── CMakeLists.txt
    └── src/
```

この場合、`qt-app/AGENTS.md` に Qt / CMake 用の追加指示を書きます。

### すべてを 1 ファイルにまとめる

小さなプロジェクトでは、`AGENTS.md` に CMake 用の指示を直接書いても構いません。

```text
your-project/
├── AGENTS.md
└── CMakeLists.txt
```

ただし、複数プロジェクトで同じ CMake 方針を使う場合は、`AGENTS.cmake.md` をホーム
に分けたほうが更新しやすくなります。

### ファイル名を変える場合

別名のファイルを使う場合は、`AGENTS.md` 側の参照文を必ず実際のファイル名に合わせ
てください。

例:

```markdown
`CMakeLists.txt` を新規作成または更新するときは、必ず `./docs/CMakeRules.md` を読んで従うこと。
```

## 運用上の注意

- `AGENTS.cmake.md` を更新したら、複数リポジトリに影響する可能性があります。
- 既存プロジェクトのビルド方針が強い場合は、そのプロジェクトの `AGENTS.md` で例外
  を明示してください。
- Qt 6.3 未満やバージョンなし Qt CMake コマンドが無効な環境では、対応パターンを
  Codex への依頼文でも明示すると意図が伝わりやすくなります。
