# docs/ README

このファイルは、`docs/` ディレクトリの内容を案内するための README です。

この `docs/` だけを渡された場合でも、まず [READING_ORDER.md](READING_ORDER.md) から読めば、全体像と読む順番が分かるようにしている。

ここでは、`docs/` 配下にどのような文書があり、何のために置かれているかを説明する。

## docs/ に置く文書

- `docs/guides/USER_GUIDE-<version>.md`
  - バージョンごとの利用ガイド
- `docs/releases/ReleaseNote-<version>.md`
  - バージョンごとのリリースノート
- `docs/history/RELEASE_HISTORY.md`
  - 全体の変更履歴
- `docs/foundation/*.md`
  - 準備基盤や更新規則の設計書
- `docs/foundation/GENERATION_INSTRUCTION.md`
  - 開発基盤文書の生成指示文
- `docs/foundation/qeyes/AGENTS.md`
  - `qeyes` の開発基盤設計文書生成ルール
- `docs/request/*.md`
  - プロジェクトの発端となる要望書
- `docs/request/GENERATION_INSTRUCTION.md`
  - 要望書から次工程の文書を生成する指示文
- `docs/request/qeyes/AGENTS.md`
  - 要望書から `qeyes` 用文書を生成するルール
- `docs/requirements/*.md`
  - アプリの要件定義書
- `docs/requirements/GENERATION_INSTRUCTION.md`
  - 要件定義書の生成指示文
- `docs/requirements/template/*.md`
  - 要件定義書テンプレート
- `docs/requirements/qeyes/AGENTS.md`
  - `qeyes` の要件定義書生成ルール
- `docs/architecture/*.md`
  - アプリ全体の概要設計書
- `docs/architecture/GENERATION_INSTRUCTION.md`
  - 概要設計書の生成指示文
- `docs/architecture/template/*.md`
  - 概要設計書テンプレート
- `docs/architecture/qeyes/AGENTS.md`
  - `qeyes` の概要設計書生成ルール
- `docs/design/*.md`
  - アプリ本体の詳細設計書
- `docs/design/GENERATION_INSTRUCTION.md`
  - 詳細設計書群の生成指示文
- `docs/design/template/*.md`
  - 詳細設計書テンプレート
- `docs/design/qeyes/AGENTS.md`
  - `qeyes` の詳細設計書生成ルール
- `docs/implementation/*.md`
  - 設計書からコードとビルド構成へ落とし込む実装工程文書
- `docs/implementation/GENERATION_INSTRUCTION.md`
  - 実装ファイルの生成指示文
- `docs/implementation/template/*.md`
  - 実装工程のテンプレート
- `docs/implementation/qeyes/AGENTS.md`
  - `qeyes` の実装ファイル生成ルール
- `docs/verification/*.md`
  - 実装成果物の確認方法を定義する検証工程文書
- `docs/verification/GENERATION_INSTRUCTION.md`
  - 検証結果の生成指示文
- `docs/verification/template/*.md`
  - 検証工程のテンプレート
- `docs/verification/qeyes/AGENTS.md`
  - `qeyes` の検証ルール
- `docs/consulting/README.md`
  - `docs/consulting/` ディレクトリーの案内
- `docs/consulting/*.md`
  - 開発実行段階ごとの AI 活用コンサルティング結果
- `BUILD_RESULT.md` / `VERIFICATION_RESULT.md` / `ISSUES.md`
  - 再現先の `qeyes/` 直下に出力される検証成果物
- `docs/READING_ORDER.md`
  - どの文書からどの順に読めばよいかを示す案内
- `docs/guides/USER_GUIDE-template.md`
  - 利用ガイド作成用テンプレート
- `docs/releases/ReleaseNote-template.md`
  - リリースノート作成用テンプレート
- `docs/*.md`
  - 補助的な運用メモや判断記録

この README は、`docs/` の入口として文書の種類と役割を簡潔に示すためのものです。
