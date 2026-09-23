# AGENTS

## 役割

このディレクトリの `AGENTS.md` は、空の `qeyes/` ディレクトリから再現するときに、`CMakeLists.txt`、`main.cpp`、`qeyes.rc`、`assets/`、`AGENTS.md` を入力として、`qeyes` の実装結果を検証するためのルールを定義する。

## 入力

- `CMakeLists.txt`
- `main.cpp`
- `qeyes.rc`
- `assets/`
- `AGENTS.md`

## 出力

- `BUILD_RESULT.md`
- `VERIFICATION_RESULT.md`
- `ISSUES.md`

## ルール

- `CMakeLists.txt` に基づいて構成生成とビルド可否を確認する。
- `main.cpp` の主要動作が `AGENTS.md` の実装方針に沿っているか確認する。
- `qeyes.rc` と `assets/` の参照が壊れていないか確認する。
- 起動後の基本操作、描画、設定保存を優先して確認する。
- 必要に応じて `BUILD_RESULT_TEMPLATE.md`、`VERIFICATION_RESULT_TEMPLATE.md`、`ISSUES_TEMPLATE.md` を参照する。
- ビルド確認結果は `BUILD_RESULT.md` に記録する。
- 実行確認結果は `VERIFICATION_RESULT.md` に記録する。
- 不整合が見つかった場合は、失敗内容と修正対象を `ISSUES.md` に記録する。
- パスは `qeyes/` ディレクトリ直下を基準に解釈する。
