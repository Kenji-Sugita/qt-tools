# AGENTS

## 役割

このディレクトリの `AGENTS.md` は、空の `qeyes/` ディレクトリから再現するときに、`OVERVIEW_DESIGN.md` を入力として詳細設計書群を生成または更新するためのルールを定義する。

## 入力

- `OVERVIEW_DESIGN.md`

## 出力

- `PRODUCT_DESIGN.md`
- `UI_DESIGN.md`
- `RENDERING_DESIGN.md`

## ルール

- 機能の責務や状態は `PRODUCT_DESIGN.md` に整理する。
- 画面、操作、ダイアログは `UI_DESIGN.md` に整理する。
- 描画要素、配置計算、追従、再描画は `RENDERING_DESIGN.md` に整理する。
- 重複を避け、各文書の役割を混同しない。
- 必要に応じて `UI_DESIGN_TEMPLATE.md` を参照する。
- パスは `qeyes/` ディレクトリ直下を基準に解釈する。
