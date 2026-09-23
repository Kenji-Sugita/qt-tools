# AGENTS

## 役割

このディレクトリの `AGENTS.md` は、空の `qeyes/` ディレクトリから再現するときに、`PRODUCT_DESIGN.md`、`UI_DESIGN.md`、`RENDERING_DESIGN.md`、`AGENTS.md` を入力として、`qeyes` の実装ファイルとビルド構成を生成または更新するためのルールを定義する。

## 入力

- `PRODUCT_DESIGN.md`
- `UI_DESIGN.md`
- `RENDERING_DESIGN.md`
- `AGENTS.md`

## 出力

- `CMakeLists.txt`
- `main.cpp`
- `qeyes.rc`
- `assets/`

## ルール

- `AGENTS.md` のアプリ実装方針に従い、`Qt6::Widgets` と `QPainter` を前提に実装する。
- `PRODUCT_DESIGN.md` の機能責務を、ウィンドウ操作、設定、終了動作へ反映する。
- `UI_DESIGN.md` のダイアログ仕様と操作仕様を UI 実装へ反映する。
- `RENDERING_DESIGN.md` の描画要素、追従、再描画仕様を描画コードへ反映する。
- リソースは `AGENTS.md` のリソース運用方針に従って `assets/` 配下へ配置する。
- 必要に応じて `IMPLEMENTATION_TEMPLATE.md` を参照する。
- パスは `qeyes/` ディレクトリ直下を基準に解釈する。
