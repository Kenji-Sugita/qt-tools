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
- 今回依頼された文書の英語化は完了。実装の変更はない。

## 次にやること

- 今回の依頼に関する追加作業はない。次回は新たな依頼内容と Git の状態を確認して着手する。

## 未完了

- 今回の依頼範囲ではなし。

## 触るファイル

- 現時点で次回の変更予定はなし。

## 注意

- README 更新時と翻訳文書の継続方針は `DECISIONS.md` を参照する。
- 今回の翻訳対象は README.en.md の直接のリンク先。各ガイド内で参照する補助文書まで再帰的には翻訳していない。

## 関連ファイル

- `README.md` / `README.en.md`: リポジトリー全体の案内とツール一覧。
- 各ツールの `README.en.md` / `USER_GUIDE.en.md`、`cling/README.en.txt`: 日本語文書に対応する英語版。
- `qmlprofileranalyzer/USER_GUIDE.md` / `qmlprofileranalyzer/USER_GUIDE.en.md`: QML Profiler トレース解析・比較・形式変換の利用方法。
- `DECISIONS.md`: 継続する判断、仕様、運用制約。
- `backlog.md`: 長期の残作業、問題点、保留事項。
