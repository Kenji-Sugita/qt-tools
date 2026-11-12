# DECISIONS

## 採用済み

- 配布用 zip のトップディレクトリは `tl/` とする。
- 配布用 zip には、ソース、`CMakeLists.txt`、利用者向け文書、テストだけを含める。
- 配布用 zip には、`build/`、`NEXT.md`、`backlog.md`、`DECISIONS.md` を含めない。

## 不採用

- なし

## まだ決めていないこと

- なし

## 判断基準

- なし

## 継続する仕様・運用制約

- `USER_GUIDE.md` から利用者ガイド PDF を作る場合は、`genpdf --format book --output USER_GUIDE.pdf USER_GUIDE.md` を使う。
