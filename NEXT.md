# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。
このファイルは、コピー先で次回再開に必要な最小限の情報へ置き換えて使う。

## 現在の状態

- 2026-11-09: 利用者の依頼で、Sites正本の更新をGitHub公開用へ同期した。着手時のローカルは古かったため、`git fetch origin` とfast-forwardでGitHubの `915e678` を取得してから比較した。
  - `work-templates/` の11月5日の更新はGitHubに反映済みで、今回の追加変更は不要。テンプレートと改善履歴はSites正本と一致し、公開用の汎用パス・英語版・ガイド補足は保持した。
  - `app-generation-workflow/` を0.2.3へ同期。レビュー・手戻り手順、記録テンプレート、事例文書、ガイドMD/PDFを反映し、既存の日英README・ガイドを整合した。
  - `whiteboard/whiteboard-app/` を配布中の1.3.0へ同期。通常保存・復元失敗の保護、保存再試行、終了確認、MCP保存状態、関連テストとガイドを反映し、新版のソースZIPを追加した。
  - `dateorder/` は11月8日の改訂ガイドMD/PDFと配布ZIP、`dtree/` は1.0.1の実装・版番号・履歴・ガイドMD/PDF・ZIPを反映。`qeyes/` はmacOS復帰処理と起動・操作説明を同期した。各英語ガイドも同じ差分に追随した。
  - `icpp/` はSitesの修復済み69ページPDFと配布ZIPを同期。旧ZIPとの内容差はPDF1件だけで、他の284ファイルは一致。実装とQt Creatorガイド0.3.1は保持した。
  - 正本ZIPと公開用の対応ファイルを全バイト照合し、ワークフロー38件・Whiteboard122件・dateorder4件・dtree5件・qeyes21件の一致、各ZIPのCRCを確認。PDF・配布ZIPは再生成せず正本を使用した。
  - macOS arm64 / Qt 6.12.0でWhiteboardとqeyesを公開用ツリーから一時ディレクトリーへビルド。WhiteboardはoffscreenでCTest 19件が成功。dtreeは構文・版表示・除外・件数上限・日時ソート・隠し項目・symlink非再帰・不正引数を確認。日英の変更箇所を独立レビューし、差分の空白も確認した。
  - qeyesのCommand-Tab・モーダルダイアログの実機操作、Windowsなど別OS、実ディスク障害、PDF再生成・全ページ目視を今回やり直したものではない。ワークフローの実案件での運用・自動反復も未検証。
  - Sitesの旧版ZIPにしかない差分、BOMのみの差分、未収録であるだけのPDFは追加しなかった。全20ツールの対応を確認した結果と、保留すべき版の差は `backlog.md` を参照。Sites自体と既存の旧版ZIPは変更していない。

- 2026-11-05: 正本 `/Users/sugita/Sites/work-templates` の更新を反映済み。`NEXT_HISTORY.md` と日英 README の案内は `192e712` に収録済み。
- 共通ルールの「作業の区切りでの案内」は、日本語版を `073c5ba`、英語版と日英利用ガイドを `192e712` で同期済み。
- 公開用の汎用コピー手順と既存の英語版・利用ガイドを維持した。改善履歴は新規プロジェクトへコピーするテンプレートには含めない。
- 今回の同期作業に未完了はない。未追跡の `release-order.txt` は今回の対象外として保持する。

- 2026-09-29: 日英 README の分類を各ツールの説明と照合し、全20ディレクトリーの掲載と日英の分類一致を確認した。
- 日英 README の分類整理を完了。WhiteboardApp を「Documents, Images and Diagrams」に配置し、「Presentation and Seminar Tools」から参照を追加した。汎用コマンドは「General-purpose Command-line Utilities」、デスクトップアクセサリーは「Desktop Accessories」に整理した。分類の判断理由は `DECISIONS.md` を参照する。
- 全20項目の重複・欠落がないこと、各項目のリンク先の存在、追加した分類内リンクの見出し、`git diff --check` を確認済み。今回の分類整理に未完了の作業はない。
- 以前未コミットだった README と `LaserPointer/` は追跡・コミット済み。今回の確認では、既存の未追跡ファイルは `release-order.txt` のみ。

- README.en.md を最新の README.md に沿って再翻訳し、Windows とコマンド系ツールの対象環境を反映済み。
- 日英 README に qmlprofileranalyzer の概要と利用ガイドへのリンクを追加済み。
- qmlprop と qmlprofileranalyzer を両 README の「QML Tools」にまとめた。
- README.en.md から直接リンクする日本語文書18件の英語版を作成し、リンク先を切り替え済み。icpp と VuStereo は既存の英語文書を利用する。
- 英語版の見出し構成、コードブロック、リンク先の存在、書式を確認済み。`tl` の日本語入力例は英語の説明を添えて保持した。
- 追加依頼により、英語版がなかった `USER_GUIDE.md` 8件に対応する `USER_GUIDE.en.md` を作成済み。対象は `cling`、`icpp`、`propertyeditor`、`qeyes`、`qmlprop`、`vu-stereo`、`whiteboard/app-generation-workflow`、`work-templates`。
- 追加8件も原文との見出し構成、コードブロック、画像・文書リンク、空白エラーを確認済み。`icpp` の日本語表示例・翻訳教材は英語の説明を添えて保持した。
- `qeyes/README.en.md` に英語利用ガイドへのリンクを追加済み。
- `work-templates` に `AGENTS.next.en.md`、`NEXT.en.md`、`DECISIONS.en.md`、`backlog.en.md` を追加済み。既存の日本語版とディレクトリー構成は保持した。
- `work-templates/README.en.md` の個人環境依存パスを除去済み。英語利用ガイドにも案内を追加した。コピー手順は日本語版に合わせて `WORK_TEMPLATES_DIR` で配置先を指定する方式へ更新し、作業対象のディレクトリーで英語版を標準ファイル名へコピーする。
- 英語テンプレート4件の見出し対応、README のリンク・コピー先、日本語版が未変更であること、空白エラーを確認済み。
- トップの日英 README の「Documentation」末尾に、ルート3ファイルが maintainer の実運用用コンテキストであること、複数ホスト・AI セッション間の共有のため Git 管理していること、再利用用テンプレートは `work-templates/` にあることを追記済み。
- 今回依頼された文書の英語化は完了。実装の変更はない。
- `work-templates/README.md` の個人環境依存パスはコミット `93b3f96` で汎用化済み。`WORK_TEMPLATES_DIR` に指定するのは `work-templates` ディレクトリーの配置先であることを明確にし、英語 README にも反映済み。

## 次にやること

- 今回の依頼に関する追加作業はない。次回は新たな依頼内容と Git の状態を確認して着手する。

## 未完了

- 今回の依頼範囲ではなし。

## 触るファイル

- 現時点で次回の変更予定はなし。

## 注意

- README 更新時と翻訳文書の継続方針は `DECISIONS.md` を参照する。
- 翻訳対象は README.en.md の直接のリンク先、追加依頼の `USER_GUIDE.md` 8件、および `work-templates` の英語版テンプレート4件。各ガイド内で参照する補助文書や版別の `USER_GUIDE-<version>.md` までは再帰的に翻訳していない。
- `whiteboard/app-generation-workflow/USER_GUIDE.en.md` は同じ場所の日本語原文（0.1.0）に対応する。別ディレクトリーの `app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md`（0.2.3）とは版が異なる。
- `work-templates` の軽量な構成と英語版の配置・コピー時の命名方針は `DECISIONS.md` を参照する。日英 README のコピー例では、`WORK_TEMPLATES_DIR` を実際の `work-templates` ディレクトリーのパスへ置き換える。

## 関連ファイル

- `work-templates/NEXT_HISTORY.md`: 正本から取り込んだテンプレートの改善履歴。文中の Git コミット番号は正本側のリポジトリーを指す。
- `work-templates/README.md` / `work-templates/README.en.md`: 正本・コピーの位置付けと汎用コピー手順。

- `README.md` / `README.en.md`: リポジトリー全体の案内とツール一覧。
- 各ツールの `README.en.md` / `USER_GUIDE.en.md`、`cling/README.en.txt`: 日本語文書に対応する英語版。
- `qmlprofileranalyzer/USER_GUIDE.md` / `qmlprofileranalyzer/USER_GUIDE.en.md`: QML Profiler トレース解析・比較・形式変換の利用方法。
- `DECISIONS.md`: 継続する判断、仕様、運用制約。
- `backlog.md`: 長期の残作業、問題点、保留事項。
