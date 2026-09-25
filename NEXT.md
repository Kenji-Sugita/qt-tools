# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。
このファイルは、コピー先で次回再開に必要な最小限の情報へ置き換えて使う。

## 現在の状態

- README.en.md を最新の README.md に沿って再翻訳し、Windows とコマンド系ツールの対象環境を反映済み。
- 日英 README に qmlprofileranalyzer の概要と利用ガイドへのリンクを追加済み。
- qmlprop と qmlprofileranalyzer を両 README の「QML Tools」にまとめた。
- README.en.md から直接リンクする日本語文書18件の英語版を作成し、リンク先を切り替え済み。icpp と VuStereo は既存の英語文書を利用する。
- 英語版の見出し構成、コードブロック、リンク先の存在、書式を確認済み。`tl` の日本語入力例は英語の説明を添えて保持した。
- 追加依頼により、英語版がなかった `USER_GUIDE.md` 8件に対応する `USER_GUIDE.en.md` を作成済み。対象は `cling`、`icpp`、`propertyeditor`、`qeyes`、`qmlprop`、`vu-stereo`、`whiteboard/app-generation-workflow`、`work-templates`。
- 追加8件も原文との見出し構成、コードブロック、画像・文書リンク、空白エラーを確認済み。`icpp` の日本語表示例・翻訳教材は英語の説明を添えて保持した。
- `qeyes/README.en.md` に英語利用ガイドへのリンクを追加済み。
- `work-templates` に `AGENTS.next.en.md`、`NEXT.en.md`、`DECISIONS.en.md`、`backlog.en.md` を追加済み。既存の日本語版とディレクトリー構成は保持した。
- `work-templates/README.en.md` の個人環境依存パスを除去し、リポジトリールートから英語版を標準ファイル名へコピーする手順に修正済み。英語利用ガイドにも案内を追加した。
- 英語テンプレート4件の見出し対応、README のリンク・コピー先、日本語版が未変更であること、空白エラーを確認済み。
- トップの日英 README の「Documentation」末尾に、ルート3ファイルが maintainer の実運用用コンテキストであること、複数ホスト・AI セッション間の共有のため Git 管理していること、再利用用テンプレートは `work-templates/` にあることを追記済み。
- 今回依頼された文書の英語化は完了。実装の変更はない。

## 次にやること

- 今回の依頼に関する追加作業はない。次回は新たな依頼内容と Git の状態を確認して着手する。

## 未完了

- 今回の依頼範囲ではなし。

## 触るファイル

- 現時点で次回の変更予定はなし。

## 注意

- README 更新時と翻訳文書の継続方針は `DECISIONS.md` を参照する。
- 翻訳対象は README.en.md の直接のリンク先、追加依頼の `USER_GUIDE.md` 8件、および `work-templates` の英語版テンプレート4件。各ガイド内で参照する補助文書や版別の `USER_GUIDE-<version>.md` までは再帰的に翻訳していない。
- `whiteboard/app-generation-workflow/USER_GUIDE.en.md` は同じ場所の日本語原文（0.1.0）に対応する。別ディレクトリーの `app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md`（0.2.2）とは版が異なる。
- `work-templates` の軽量な構成と英語版の配置・コピー時の命名方針は `DECISIONS.md` を参照する。日本語 README の個人パスは、日本語版を変更しない指定に従って保持した。

## 関連ファイル

- `README.md` / `README.en.md`: リポジトリー全体の案内とツール一覧。
- 各ツールの `README.en.md` / `USER_GUIDE.en.md`、`cling/README.en.txt`: 日本語文書に対応する英語版。
- `qmlprofileranalyzer/USER_GUIDE.md` / `qmlprofileranalyzer/USER_GUIDE.en.md`: QML Profiler トレース解析・比較・形式変換の利用方法。
- `DECISIONS.md`: 継続する判断、仕様、運用制約。
- `backlog.md`: 長期の残作業、問題点、保留事項。
