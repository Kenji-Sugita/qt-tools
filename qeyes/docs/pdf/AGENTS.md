# AGENTS

## 役割

このディレクトリの `AGENTS.md` は、`docs/` 配下の Markdown から PDF を生成または更新するときのルールを定義する。

## 基本方針

- PDF 生成には `genpdf` を使う。
- 単一の `.md` は個別に PDF 化する。
- 複数の `.md` をまとめる場合は、各 `.md` を先に個別 PDF 化してから結合する。
- 複数 Markdown を先に 1 つの Markdown に結合してから PDF 化する方式は使わない。

## 入力

- `docs/` 配下の各 `.md`
- 必要に応じて `docs/pdf-src/` の結合用中間 Markdown

## 出力

- `docs/pdf/*.pdf`

## 命名ルール

- `docs` 直下の単独文書は、元ファイル名と同じ stem の PDF にする。
  - 例: `docs/README.md` -> `docs/pdf/README.pdf`
- 工程単位の完全版は `*-complete-from-pdfs.pdf` とする。
  - 例: `design-complete-from-pdfs.pdf`
- 個別 PDF を結合する途中成果物が必要な場合は、一時的に `docs/pdf/*-parts/` を使ってよい。
- `*-parts/` は作業用であり、最終的に不要なら削除する。

## 結合順

- `request`
  - `REQUEST.md`
  - `GENERATION_INSTRUCTION.md`
  - `qeyes/AGENTS.md`
- `requirements`
  - `REQUIREMENTS.md`
  - `GENERATION_INSTRUCTION.md`
  - `qeyes/AGENTS.md`
  - `template/REQUIREMENTS_TEMPLATE.md`
- `architecture`
  - `OVERVIEW_DESIGN.md`
  - `GENERATION_INSTRUCTION.md`
  - `qeyes/AGENTS.md`
  - `template/OVERVIEW_DESIGN_TEMPLATE.md`
- `design`
  - `README.md`
  - `PRODUCT_DESIGN.md`
  - `UI_DESIGN.md`
  - `RENDERING_DESIGN.md`
  - `GENERATION_INSTRUCTION.md`
  - `qeyes/AGENTS.md`
  - `template/UI_DESIGN_TEMPLATE.md`
- `implementation`
  - `IMPLEMENTATION_DESIGN.md`
  - `GENERATION_INSTRUCTION.md`
  - `qeyes/AGENTS.md`
  - `template/IMPLEMENTATION_TEMPLATE.md`
- `verification`
  - `VERIFICATION_DESIGN.md`
  - `GENERATION_INSTRUCTION.md`
  - `qeyes/AGENTS.md`
  - `template/BUILD_RESULT_TEMPLATE.md`
  - `template/VERIFICATION_RESULT_TEMPLATE.md`
  - `template/ISSUES_TEMPLATE.md`
- `foundation`
  - `FOUNDATION_DESIGN.md`
  - `DELIVERABLES.md`
  - `MAINTENANCE_RULES.md`
  - `GENERATION_INSTRUCTION.md`
  - `SETUP_DESIGN_NOTE.md`
  - `qeyes/AGENTS.md`

## 更新ルール

- 元の Markdown を修正した場合は、対応する PDF も更新する。
- `READING_ORDER.pdf` を更新するときは、`docs/pdf` に現在ある PDF の読む順番と一致していることを確認する。
- 工程単位の complete PDF を更新するときは、対応する単独 PDF 群も必要に応じて更新する。
- 不要になった旧方式の PDF は残さない。
