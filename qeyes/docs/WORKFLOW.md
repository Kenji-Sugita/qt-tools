# 運用ルールの伝え方

`AGENTS.md` だけに寄せるより、役割ごとに分けた方が運用しやすいです。

## 有効な方法

- `AGENTS.md`
  - AI や作業者向けの実務ルールを書く
  - 例: 文書配置、命名、更新手順
- `docs/README.md`
  - 人が見て分かる文書構成の説明を書く
  - 今回のような `docs/` 運用には相性が良い
- テンプレート化
  - `docs/releases/ReleaseNote-template.md`
  - `docs/guides/USER_GUIDE-template.md`
  - 新版を作るたびに迷わない
  - カレントディレクトリー名やプロジェクト名をファイル名に混ぜない判断を固定しやすい
- PR テンプレート
  - 新バージョン時に更新すべき項目をチェックリスト化する
  - 例: バージョン番号、履歴、ガイド、リリースノート、タグ
- CI / lint
  - 命名規則や必須ファイルの存在を自動確認する
  - 例: `ReleaseNote-x.y.md` があるのに `USER_GUIDE-x.y.md` が無ければ失敗
- Makefile / script
  - `./scripts/new_release_doc.sh 1.2` のように雛形生成を自動化する
- Git tag 運用の明文化
  - `README` や `docs/README.md` に「版の正本はタグで追う」と書く

## 実務上のおすすめ

次の組み合わせが実用的です。

1. `AGENTS.md` に短い強いルールを書く
2. `docs/README.md` に人向けの説明を書く
3. テンプレートを置く
4. 可能なら CI で崩れを検知する

理由は、`AGENTS.md` だけだと「読めば分かる」止まりで、守られないことがあるためです。テンプレートや CI があると、運用が規則依存ではなく仕組み依存になります。

## このリポジトリでの優先順

このリポジトリでは、まず次の順で整備するのが現実的です。

1. `AGENTS.md` に短いルールを書く
2. `docs/README.md` を維持する
3. `docs/releases/ReleaseNote-template.md` を追加する
4. `docs/guides/USER_GUIDE-template.md` を追加する
