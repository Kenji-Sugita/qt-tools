# 準備基盤設計

## 目的

`qeyes` を継続的に実装・保守・リリースできる状態を整えるため、作業ルール、文書構成、リソース運用、版管理の基盤を定義する。

## 対象

- 実装方針
- 文書方針
- リソース方針
- README 方針
- リリース時の基礎運用
- テンプレート整備

## 対象外

- 個別機能の詳細仕様
- UI の細部デザイン
- アルゴリズムの最適化方針
- 各プラットフォーム固有の実装詳細

## 完了条件

次の条件を満たしたとき、この準備基盤は整ったものとみなす。

- `AGENTS.md` が存在する
- アプリ実装方針が定義されている
- リソース運用方針が定義されている
- 文書運用方針が定義されている
- ルート `README.md` が存在する
- `docs/README.md` が存在する
- `docs/history/RELEASE_HISTORY.md` が存在する
- `docs/guides/USER_GUIDE-template.md` が存在する
- `docs/releases/ReleaseNote-template.md` が存在する

## 設計方針

- ルールは `AGENTS.md` に集約する
- 利用者向けの入口はルート `README.md` に置く
- `docs/README.md` は `docs/` ディレクトリの案内に限定する
- 版付き文書は `docs/guides/` と `docs/releases/` に分離する
- 履歴は `docs/history/RELEASE_HISTORY.md` に集約する
- 過去版の完全な状態は Git タグで追跡する

## 将来拡張

次のような文書種別が増える場合は、この基盤設計の延長として扱う。

- `docs/architecture/`
- `docs/design/`
- `docs/testing/`

その場合は、`docs/README.md` と `AGENTS.md` の両方を更新する。
