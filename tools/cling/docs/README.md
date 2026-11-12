# docs README

`docs/` 配下の Markdown 文書の概要です。`Qt Cling` のリリース方針、対象読者、支援ツール案、Qt 開発や単体テストへの適用アイデアを整理したメモをまとめています。

## 読み始めの目安

全体像を先に把握したい場合は、次の順で読むと流れを追いやすいです。

1. `release-plan.md`
2. `target-audience.md`
3. `experimental-release-plan.md`
4. `qtcreator-plugin-ideas.md`
5. `qtcling-support-tools-priority.md`

## Markdown 一覧

### 企画・リリース方針

- `release-plan.md`: 初回リリースを `Qt Cling CLI` 中心で進める理由と、段階的なロードマップを整理した方針メモです。
- `experimental-release-plan.md`: macOS 向け実験的バイナリ配布を前提に、配布物へ含めるべきファイルや最小構成を整理しています。
- `internal-announcement-draft.md`: 部内向けの案内文案です。ツールの概要、想定ユーザー、今後の課題を短く説明しています。

### 対象ユーザー・適用範囲

- `target-audience.md`: 初期ターゲット層、提供価値、向いている作業と向いていない作業を整理したメモです。
- `windows-native-arm64.md`: Windows 11 ARM64ネイティブ開発版の構成、ビルド、起動、確認済み機能、プラットフォーム固有修正をまとめています。
- `gui-library-candidates.md`: 現在の Qt GUI REPL の仕組みを前提に、Qt 以外の GUI ライブラリーへ展開できる候補を比較しています。
- `ToDo.md`: `Cling` のインストール手順、成功したブランチ、用途、改善項目をまとめた作業メモです。

### 支援ツール・IDE 連携案

- `qtcling-support-tools-priority.md`: `Qt Cling` 向け支援ツール案を優先度順に並べ、判断軸と狙いを説明しています。
- `qtcreator-plugin-ideas.md`: `Qt Creator` プラグインとして展開する場合の方向性と、学習支援を含む機能案のメモです。

### サンプル・断片メモ

- `samples-repl.md`: `QVariant` を題材にした短い REPL 実行例です。

### Qt 開発適用メモ

- `qtdev/qtdev1.md`: `Cling` を Qt 開発へ適用する全体像を、Widgets と QML の両面から広く洗い出したメモです。
- `qtdev/qtdev2.md`: Widgets 向け、QML 向け、共通テーマ向けの具体的なサンプル案を多数列挙しています。
- `qtdev/qtdev3.md`: 単体テストの前段や補助用途として `Cling` をどう使うかを整理したメモです。
- `qtdev/qtdev4.md`: Qt 単体テスト支援に使える具体的な題材を、検証観点付きでまとめています。
- `qtdev/qtdev5.md`: `Cling` での実験コードと、正式な `Qt Test` へ落とし込む対応関係を整理しています。
- `qtdev/qtdev6.md`: 共通ヘルパーを含むサンプル集の構成案をまとめたメモです。実験からテスト化への導線を意識しています。

## 補足

- `docs/tutorial/` にはチュートリアル用のソースコードがありますが、この README では Markdown 文書のみを対象にしています。
- `docs/announce`、`docs/descdev`、`docs/descdev2`、各 `memo` は Markdown ではないため一覧から外しています。
