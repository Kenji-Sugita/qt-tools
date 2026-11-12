# 更新ルール

## 目的

文書やルールが増えたときに、どこを更新すべきかを判断できるようにする。

## 基本ルール

- 既存種別の文書を追加した場合は、必要に応じて `docs/history/RELEASE_HISTORY.md` を更新する
- 新しい文書種別を追加した場合は、`docs/README.md` にその種別と役割を追記する
- 利用者が参照すべき主要文書を追加した場合は、ルート `README.md` のリンクも更新する
- 文書配置や命名ルール自体に影響する場合は、`AGENTS.md` も更新する
- 文書を追加した後は `git status` を確認し、未追跡のまま残っていないことを確認する
- 他ホストで参照させる前に、必要な文書が add・commit・push 済みであることを確認する

## 工程ごとの修正ルール

- 各工程で問題が起きた場合は、まずその工程の文書を修正対象とする
- 修正対象には、その工程の入力文書、テンプレート、`AGENTS.md`、`GENERATION_INSTRUCTION.md` を含めて確認する
- `request` の問題は `REQUEST.md` と `docs/request/qeyes/AGENTS.md` を中心に見直す
- `requirements` の問題は要件定義書、要件テンプレート、`docs/requirements/qeyes/AGENTS.md` を中心に見直す
- `architecture` の問題は概要設計書、概要設計テンプレート、`docs/architecture/qeyes/AGENTS.md` を中心に見直す
- `design` の問題は詳細設計書、詳細設計テンプレート、`docs/design/qeyes/AGENTS.md` を中心に見直す
- `implementation` の問題は実装工程文書と `docs/implementation/qeyes/AGENTS.md` を中心に見直す
- `verification` の問題は検証工程文書と `docs/verification/qeyes/AGENTS.md` を中心に見直す
- 問題が上流工程に起因すると判断できる場合は、その上流工程まで戻って修正する

## README の役割分担

- `README.md`
  - プロジェクト全体の説明
- `docs/README.md`
  - `docs/` ディレクトリの案内

## 判断表

- 既存種別の版付きガイドを追加する
  - `docs/guides/USER_GUIDE-<version>.md`
  - `docs/history/RELEASE_HISTORY.md`
  - 必要なら `README.md`
- 既存種別の版付きリリースノートを追加する
  - `docs/releases/ReleaseNote-<version>.md`
  - `docs/history/RELEASE_HISTORY.md`
  - 必要なら `README.md`
- 新しい文書種別を追加する
  - `docs/README.md`
  - `AGENTS.md`
  - 必要ならテンプレート
- ルール文書を追加する
  - `docs/README.md`
  - `AGENTS.md`
- README の役割を変える
  - `README.md`
  - `docs/README.md`
  - `AGENTS.md`

## 非目標

- 個別機能の仕様判断
- 実装コードの設計判断
- UI デザインの最終判断
