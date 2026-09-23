# 成果物一覧

## 目的

準備基盤によって生成・維持される成果物を明確化する。

## 主要成果物

- `AGENTS.md`
  - 実装、文書、リソース、README の運用ルール
- `README.md`
  - プロジェクト全体の入口
- `docs/README.md`
  - `docs/` ディレクトリの案内
- `docs/history/RELEASE_HISTORY.md`
  - 変更履歴
- `docs/guides/USER_GUIDE-<version>.md`
  - 版付き利用ガイド
- `docs/releases/ReleaseNote-<version>.md`
  - 版付きリリースノート

## テンプレート

- `docs/guides/USER_GUIDE-template.md`
  - 利用ガイド作成用テンプレート
- `docs/releases/ReleaseNote-template.md`
  - リリースノート作成用テンプレート

## 補助文書

- `docs/*.md`
  - 運用メモ、判断記録、調査結果
- `docs/foundation/*.md`
  - この準備基盤自体の設計書

## 相互参照

- `README.md` は主要文書への導線を持つ
- `docs/README.md` は `docs/` 配下の文書種別を示す
- `AGENTS.md` は各成果物の作成・更新ルールを定義する
- 版付き文書の追加時は `docs/history/RELEASE_HISTORY.md` を更新する
