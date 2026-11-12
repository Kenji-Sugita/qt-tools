# AGENTS

## 役割

このディレクトリの `AGENTS.md` は、空の `qeyes/` ディレクトリから再現するときに、`REQUIREMENTS.md` を入力として `OVERVIEW_DESIGN.md` を生成または更新するためのルールを定義する。

## 入力

- `REQUIREMENTS.md`

## 出力

- `OVERVIEW_DESIGN.md`

## ルール

- 要件定義を全体構成、主要責務、データの流れ、非機能方針に整理する。
- 詳細実装や UI 部品単位の細部には踏み込みすぎない。
- 詳細設計に分かれるべき内容は概要レベルに留める。
- 必要に応じて `OVERVIEW_DESIGN_TEMPLATE.md` を参照する。
- パスは `qeyes/` ディレクトリ直下を基準に解釈する。
