# アプリ作成ワークフロー

このディレクトリは、仕様化前ノートと三点セットを使って、アプリ作成を実装前に整理するための汎用ワークフローである。

人が使う場合は、まず次の文書を読む。

- `USER_GUIDE.md`

AI に作業前提を理解させる場合は、次の文書を読ませる。

- `three_key_documents_workflow.md`

コード生成後の確認、設計文書化、運用判断には、次の文書を使う。

- `docs/post_generation_workflow.md`

その後、`templates/` 配下のテンプレートを使って、次の順番で作業する。
ワークフローは、ユーザーの作業場所にコピー済みのものだけを参照し、他の場所にあるワークフローは参照しない。
対象アプリ固有の仕様、実装プロンプト、ソースコードは、新しく作成する対象アプリ用ディレクトリ配下に置く。

```text
仕様化前ノートと三点セットの理解
  ↓
任意形式の要求 .txt
  ↓
要求定義レビュー
  ↓
必要なら requirement.txt 修正
  ↓
対象アプリ用ディレクトリ作成
  ↓
要求 .txt から仕様化前ノート作成
  ↓
三点セット
  ↓
必要なら共通実装制約
  ↓
実装プロンプト
  ↓
アプリ実装
  ↓
コード生成後のテスト作成・確認
  ↓
実装結果レビュー
  ↓
必要なら仕様への反映
  ↓
実装設計契約
  ↓
アプリ設計書
  ↓
アプリ設計書レビュー
  ↓
仕様への反映確認
```

実装結果の修正が必要な場合は、下流のコードから直さず、上流へ戻る。

```text
意図違いの指摘
  ↓
仕様化前ノート修正
  ↓
三点セット修正
  ↓
実装プロンプト修正
  ↓
コード修正
```

## ファイル構成

```text
app-generation-workflow/
├─ README.md
├─ USER_GUIDE.md
├─ three_key_documents_workflow.md
└─ templates/
   ├─ requirement.txt
   ├─ source_memo.md
   ├─ source_memo_reverse_review.md
   ├─ 01_usecase_spec.md
   ├─ 02_ui_spec.md
   ├─ 03_business_spec.md
   ├─ implementation_constraints.md
   └─ implementation_prompt.md
```

`requirement.txt` は、仕様化前ノートを作る前に要求を整理するための推奨テンプレートである。
任意形式の要求でもよいが、未決事項を減らしたい場合はこのテンプレートを使う。
`source_memo.md` は、要求 `.txt` から新規に仕様化前ノートを作る場合に使う。
`source_memo_reverse_review.md` は、既存実装、生成済み三点セット、またはコードから仕様化前ノートを再構成する場合に使う。
逆生成では、実装から読み取れる挙動をそのまま正式仕様にせず、現行実装から読み取れる挙動、仕様として採用する挙動、バグ候補・要確認事項を分けて整理する。
`implementation_constraints.md` は、分割した仕様化ワークフロー全体に共通する実装制約を置く任意テンプレートである。
三点セットには混ぜず、実装プロンプト作成時に参照する。
実装プロンプトでは、実装時の作業ディレクトリを対象アプリ用ディレクトリとして明記する。
`implementation_design_contract.md` は、実装結果レビュー後に三点セットと実装コードの対応を記録する文書である。
未解決の重大な不整合がある場合は契約化しない。
`app_design.md` は、実装設計契約作成後にアプリ全体の設計を記録する文書である。
三点セット、実装設計契約、実装コードの整合をレビューしてから後続判断に使う。

## 使い方

人が使う場合:

```text
USER_GUIDE.md を読んでください。
```

AI に作業前提を理解させる場合:

```text
app-generation-workflow/three_key_documents_workflow.md を読んでください。
```

`three_key_documents_workflow.md` は作業手順の正本である。
