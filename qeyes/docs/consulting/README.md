# consulting/ README

このファイルは、`docs/consulting/` ディレクトリーの内容を案内するための README です。

`CONSULTING.md` をもとに、Qt アプリケーション開発で生成 AI をどの工程にどう適用し、人がどこで介入すべきかを整理した結果を配置します。

## consulting/ に置く文書

- `CONSULTING_SUMMARY.md`
  - 開発実行段階を横断したコンサルティング結果の要約
- `AI_TOOL_SELECTION.md`
  - 採用する AI 関連ツールと補足的な連携機能の整理
- `REQUEST_TO_REQUIREMENTS.md`
  - `request -> requirements` のコンサルティング結果
- `REQUIREMENTS_TO_ARCHITECTURE.md`
  - `requirements -> architecture` のコンサルティング結果
- `ARCHITECTURE_TO_DESIGN.md`
  - `architecture -> design` のコンサルティング結果
- `DESIGN_TO_IMPLEMENTATION.md`
  - `design -> implementation` のコンサルティング結果
- `IMPLEMENTATION_TO_VERIFICATION.md`
  - `implementation -> verification` のコンサルティング結果
- `VERIFICATION_TO_FOUNDATION.md`
  - `verification -> foundation` のコンサルティング結果

## 工程別文書に共通して含める内容

- 対象工程
- 要約
- 入力
- 出力
- AI の役割
- 人の役割
- 適用条件
- 非適用条件
- 自動化できる範囲
- 自動化しにくい範囲
- Qt 固有の重点論点
- 推奨 LLM / 使い分け
- AI と既存自動化手段の役割分担
- プロンプト設計方針
- 導入段階
- KPI
- 顧客提案時の訴求点

`CONSULTING_SUMMARY.md` は全体要約文書のため、上記の工程別文書の共通構成はそのままは適用しない。
この文書では、全体方針、適用条件、Qt 固有論点、ツールの役割分担、導入段階、KPI を横断的に整理する。

## 読み方

- 全体像を把握するときは `CONSULTING_SUMMARY.md` から読む
- 個別工程の検討では該当する遷移文書を読む
- 顧客提案へ転用するときは要約と訴求点を先に確認する
