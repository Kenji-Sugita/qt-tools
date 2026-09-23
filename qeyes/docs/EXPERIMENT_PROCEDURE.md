# 実験手順書

## 目的

空の `qeyes/` ディレクトリから始めて、`request` から `verification` までを順に流し、文書生成と実装生成の再現性を確認する。

## 前提

- 再現対象の空ディレクトリ名は `qeyes/` とする。
- 各段階では、その段階用の `AGENTS.md` を `qeyes/` 直下の `AGENTS.md` として使う。
- 各段階で問題が起きた場合は、その段階の文書、テンプレート、`AGENTS.md`、`GENERATION_INSTRUCTION.md` を見直す。

## 手順

### 初期準備

- 空の `qeyes/` ディレクトリを作成する。

### request / 要望

- `REQUEST.md` を `qeyes/` 直下に配置する。
- `docs/request/qeyes/AGENTS.md` を `qeyes/AGENTS.md` として配置する。
- `docs/request/GENERATION_INSTRUCTION.md` に従って `REQUIREMENTS.md` を生成する。
- `REQUIREMENTS.md` が要望を機能要件、非機能要件、制約、対象外、受け入れ条件に整理できていることを確認する。

### requirements / 要件定義

- `docs/requirements/qeyes/AGENTS.md` を `qeyes/AGENTS.md` に置き換える。
- 必要なら `REQUIREMENTS_TEMPLATE.md` を `qeyes/` 直下に配置する。
- `docs/requirements/GENERATION_INSTRUCTION.md` に従って `REQUIREMENTS.md` を更新または確定する。

### architecture / 概要設計

- `docs/architecture/qeyes/AGENTS.md` を `qeyes/AGENTS.md` に置き換える。
- 必要なら `OVERVIEW_DESIGN_TEMPLATE.md` を `qeyes/` 直下に配置する。
- `docs/architecture/GENERATION_INSTRUCTION.md` に従って `OVERVIEW_DESIGN.md` を生成する。
- `OVERVIEW_DESIGN.md` が全体構成、主要責務、データの流れ、非機能方針を整理できていることを確認する。

### design / 詳細設計

- `docs/design/qeyes/AGENTS.md` を `qeyes/AGENTS.md` に置き換える。
- 必要なら `UI_DESIGN_TEMPLATE.md` を `qeyes/` 直下に配置する。
- `docs/design/GENERATION_INSTRUCTION.md` に従って `PRODUCT_DESIGN.md`、`UI_DESIGN.md`、`RENDERING_DESIGN.md` を生成する。
- 詳細設計書群が役割ごとに分かれ、重複や欠落がないことを確認する。

### implementation / 実装

- `docs/implementation/qeyes/AGENTS.md` を `qeyes/AGENTS.md` に置き換える。
- 必要なら `IMPLEMENTATION_TEMPLATE.md` を `qeyes/` 直下に配置する。
- `docs/implementation/GENERATION_INSTRUCTION.md` に従って `CMakeLists.txt`、`main.cpp`、`qeyes.rc`、`assets/` を生成する。
- 実装成果物が存在し、詳細設計書と `AGENTS.md` の方針に沿っていることを確認する。

### verification / 検証

- `docs/verification/qeyes/AGENTS.md` を `qeyes/AGENTS.md` に置き換える。
- 必要なら `BUILD_RESULT_TEMPLATE.md`、`VERIFICATION_RESULT_TEMPLATE.md`、`ISSUES_TEMPLATE.md` を `qeyes/` 直下に配置する。
- `docs/verification/GENERATION_INSTRUCTION.md` に従って `BUILD_RESULT.md`、`VERIFICATION_RESULT.md`、`ISSUES.md` を生成する。
- `BUILD_RESULT.md`、`VERIFICATION_RESULT.md`、`ISSUES.md` が揃ったら、実験を一巡完了とする。

## 各段階の完了条件

- `request`
  - `REQUIREMENTS.md` の生成に必要な入力と `AGENTS.md` が揃っている。
- `requirements`
  - `REQUIREMENTS.md` が要件定義として成立している。
- `architecture`
  - `OVERVIEW_DESIGN.md` が概要設計として成立している。
- `design`
  - `PRODUCT_DESIGN.md`、`UI_DESIGN.md`、`RENDERING_DESIGN.md` が役割分担できている。
- `implementation`
  - `CMakeLists.txt`、`main.cpp`、`qeyes.rc`、`assets/` が揃っている。
- `verification`
  - `BUILD_RESULT.md`、`VERIFICATION_RESULT.md`、`ISSUES.md` が揃っている。

## 失敗時の戻り方

- 要件の粒度や内容が不適切なら `request` または `requirements` に戻る。
- 構成の切り分けが不適切なら `architecture` に戻る。
- 詳細設計の役割分担が不適切なら `design` に戻る。
- 実装のぶれや不足があれば `implementation` に戻る。
- 検証項目や確認結果の不足があれば `verification` を見直す。
- 問題の原因が上流にあると判断できる場合は、その上流工程まで戻って修正する。

## 注意

- 最初の実験は、完全成功よりも詰まる箇所の洗い出しを目的とする。
- 実験中に見つかった不足は、その段階の文書へ反映して再現性を上げる。
- `AGENTS.md` は実験中に段階ごとに置き換えるが、最終的にどの内容を正本として残すかは別途判断する。
