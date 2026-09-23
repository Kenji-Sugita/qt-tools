# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 次にやること

- 配布する場合は、最新の `USER_GUIDE.md` / `USER_GUIDE.pdf` と `--clip-green` の説明を含めて `cutter-release-20260506.zip` を作り直す。

## 未完了

- 次回の作業判断に必要な未完了だけを書く
- 詳細な残作業、問題点、保留事項は `backlog.md` に移す

- なし。

## 触るファイル

- `NEXT.md`
- `main.cpp`
- `USER_GUIDE.md`
- `USER_GUIDE.pdf`
- `testdata/README.md`
- `testdata/vu.png`
- `.gitignore`

## 注意

- `--clip-green` は、端からつながる緑系背景を検出して透明化する。不定形の対象でも、外周の緑背景から到達できない対象内の色は背景として扱わない。
- 背景判定のしきい値調整オプションは現時点では追加しない。
- `testdata/vu.png` は `--clip-green` のクロマキー確認用サンプル。
- `USER_GUIDE.pdf` は `genpdf USER_GUIDE.md` で再生成済み。
- 既存の `cutter-release-20260506.zip` は、直近の利用ガイド更新より前に作成された可能性がある。配布前に作り直す。
- `cutter-release-20260506.zip` には `NEXT.md`、`build/`、確認用生成画像は含めない。
- `cutter-release-20260506.zip` は配布用ファイルとして作業ツリーに残すが、Git 管理外。`.gitignore` の `cutter-release-*.zip` により `git status` には表示されない。
