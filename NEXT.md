# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。
このファイルは、コピー先で次回再開に必要な最小限の情報へ置き換えて使う。

## 現在の状態

- 2026-11-13: 利用者の「qt-toolsに公開」の依頼により、Sites `ee33ffd` の改訂10候補を追加収録した。
  - docsへ8資料群、toolsへfileservermcpとwarikanを追加。正本の原稿・PDF・ZIP35ファイルと、ZIP内39ファイルの内容一致を照合した（重複を除いた公開先70ファイル）。13 PDFは正本の既存配布物をそのままコピーし、公開用に再生成していない。
  - 一般業務向け練習帳は4冊・README・TRYOUT・ZIPを収録。社内向けQt関連業務練習帳は除外した。Codex安全利用は本編・Windows補足・紹介資料だけで、別のヒアリングPDF・codex.pdf・NEXT.pdfは収録しない。
  - Qt Creator MCPは19.0.0・macOS・旧HTTP+SSEの手順例。現行版の接続・操作を今回検証したとは扱わない。fileservermcpの教材上の制約、warikanの保存復元なしと過去の試行メモの位置付けを維持した。
  - 資料の日英README、索引・トップ、PUBLICATION_MAPを更新。ツール2件に英語利用ガイドを追加し、fileservermcpの82コード例はMermaidの翻訳ラベル以外を保持した。横断文書8資料群は日本語本文への英語案内であり、全文英訳ではない。
  - fileservermcpを別作業先でQt 6.12.0/macOS arm64によりビルドし、ループバック・ダミーファイルで13項目を確認。warikanは構文・Node.jsの7ロジック項目と永続化API検索を確認。ブラウザーGUI・別OS・実案件の効果測定は未実施。
  - `verification/next10-20261113/` に正本対応ハッシュ・ZIP照合・リンクとコードフェンスの確認・実行結果を保存。差分の空白は正本由来の空白と公開用の追加文書を区別する。SitesのHTML・作業記録・バックアップはコピーしない。既存のWhiteboard旧ワークフローも保持する。
  - 利用者の公開指示に基づき、この追加と索引・管理記録をコミットし、GitHub mainへpushする。push後にHEADとリモートの一致、公開ファイルのHTTP取得を確認する。

- 2026-11-13: 利用者の依頼で、Sitesで見直したAI公開候補1～3を公開用へ追加した。
  - `docs/basics/markdown-for-ai/` と `docs/basics/ai-usage-tips/` に原稿・既存PDF、`docs/safety/personal-ai-usage-rules/` に原稿を収録。正本はSitesの `6846fa7` に保存済みの2026-11-13改訂版で、本文・PDFはバイト一致のコピーとする。
  - 各資料の日英README、カテゴリーと文書一覧・トップの日英索引、`PUBLICATION_MAP.md` を更新。日本語本文への英語案内であり、本文の英訳を追加したものではない。
  - 正本側ではPDF7ページと4ページの全ページ画像確認・通常サイト検査（表示664件、エラー0・警告1）を実施済み。この公開用作業ではPDFを再生成せず、同一性を照合する。
  - 原稿3件・PDF2件の正本とのバイト一致、変更・追加文書のローカルリンク139件、コードフェンス、公開内容、差分を確認した。利用者の指示によりコミットしGitHubの `main` へpushする。既存未追跡 `release-order.txt`、Sitesの作業記録・HTML・検査設定は収録しない。
  - Whiteboardの旧ワークフローは意図的な履歴保持として変更しない。Sites正本・他ツールの実装も今回変更しない。

- 2026-11-12: 利用者の依頼で、公開用リポジトリーを `tools/`（18件）、`workflows/`（3件）、`docs/`（7カテゴリー）へ再構成した。
  - トップREADMEを3系統への入口とし、既存の詳しい紹介・用途別一覧を各系統の日英READMEへ移した。文書カテゴリーは既存資料への索引で、新規候補10件の本文はまだ追加していない。
  - 移動した3,090ファイル中3,088件はGitの内容が同一。残る2件は参照修正のみで、実装・CMake・PDF・ZIPは保持した。正本は移動せず、対応は `PUBLICATION_MAP.md` に記録した。
  - 日英の一覧（18ツール・3ワークフロー）、新規案内のローカルリンク・見出し参照、既存ファイルの保持を確認した。既存の未解決Markdownリンク70件は増えていない。
  - macOS arm64 / Qt 6.11.2でAnker Layoutとicppを新配置からビルド。Anker Layoutのoffscreen CTest 1件、icppの統合・MCP・ソース構成CTest 3件がすべて成功した。全ツール・別OS・手動GUIを再検証したものではない。
  - 利用者からコミット・pushまで依頼された。今回の構成変更と本管理記録を同じコミットで扱い、反映先はGitHubの `main` とする。既存の未追跡 `release-order.txt` は対象外として保持する。

- 2026-11-12: `work-templates` のトップ紹介と Anker Layout の公開を完了した。
  - トップの日英READMEへ「Featured: Work Templates」を追加し、3ファイルの役割、開始・更新・再開の流れ、人とAIの役割を説明した。コピー手順は既存ファイルを上書きしない案内を日英で揃えた。`c867697` でコミット・push済みで、SafariのGitHub画面でも追加節の表示を確認した。
  - 正本 `/Users/sugita/Sites/tools/anker-layout/anker-layout.zip` の27ファイルを `anker-layout/` に展開し、元ZIPも収録した。26ファイルはバイト一致し、`iccp/README.md` だけ公開用の相対パスと `go()` 呼び出しを含む手順へ調整した。実装・CMake・日本語利用ガイド・PDF・元ZIPは正本のまま保持した。
  - Anker Layoutの日英README、英語利用ガイド、対話操作例の英語READMEを追加。トップの日英READMEに「Qt Widgets Layouts」分類を設け、一覧は全21ツールになった。公開用補足と正本の扱いは `DECISIONS.md` 第10・11項を参照する。
  - macOS arm64 / Qt 6.11.2でライブラリー・6サンプル・テストの全体ビルドと `basic_form` 単体ビルドが成功。offscreenで実行したCTestは1ターゲットが成功し、その中の6振舞いテストと初期化・終了処理を合わせて8 passedだった。日英文書・リンク・配布物との対応・`git diff --check` も確認した。
  - Anker Layoutは `1b92fa2` でコミット・push済み。SafariでGitHubトップの分類と、`anker-layout/` のREADME・配布物の掲載を確認した。GUIの手動操作、対話REPLでの実行、別OS、PDF再生成・全ページ表示確認は未実施で、追加検証の条件は `backlog.md` を参照する。
  - この時点では管理記録更新は未コミット・未pushだった。後続の構成変更と合わせ、コミット・push依頼の対象に含めた。既存の未追跡 `release-order.txt` は対象外として保持する。

- 2026-11-12: 利用者の依頼で、トップの日英READMEに「Featured: App Generation Workflow」を追加した。
  - Safariの公開画面で `main` の最新コミット `db5f779` を確認。ローカルを `git fetch origin` とfast-forwardで同コミットへ更新してから、最新のワークフロー0.2.3を参照した。
  - Qt Cling／icppなどの紹介に続けて、要求整理から実装・テスト・設計文書化までの流れ、UC・UI・ビジネスレイヤーの三点セット、「ファイルを開く」の説明例、人とAIの役割、修正時の戻り先、始め方を追加した。
  - 独立レビューで原資料との整合と日英対応を確認。コピー元は内側の `app-generation-workflow/app-generation-workflow/`、コピー先は作業場所の `app-generation-workflow/` と明記した。紹介の継続方針は `DECISIONS.md` 第9項を参照する。
  - 参照したワークフロー本体のREADME・利用ガイド・`three_key_documents_workflow.md`・`review_and_rework.md` はSites正本とバイト一致を確認。今回の編集対象はトップREADME2件で、正本やワークフロー本体・PDF・ZIPは変更していない。
  - 追加節以外のREADME本文がHEADと一致すること、各READMEのローカルリンク26件、全20ツールの重複・欠落がないこと、コードフェンス、`git diff --check` を確認した。今回アプリ生成・ビルド・実機試験は行っていない。
  - 利用者のpush依頼を受け、README2件と管理記録2件を `8b4a325` でコミットし、GitHubの `main` へpushした。SafariでトップREADMEを再読込し、追加節・導入文・工程図・三点セットの表の表示を確認した。英語版も同コミットに収録したが、英語ページの描画確認は未実施。既存の未追跡 `release-order.txt` は変更せず保持した。

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

- アプリ生成ワークフローと `work-templates` のREADME紹介、Anker Layoutの公開はGitHubへ反映済み。今回の公開依頼に追加作業はない。
- 次の依頼時はGitの状態と該当ツールのSites正本を確認する。追加の実機・環境別検証を行う場合は `backlog.md` の条件を参照する。

## 未完了

- 構成変更の実装・検証は完了。GitHubへの反映結果は今回のコミットとリモートの `main` で確認する。未解決リンクなど既存の課題は `backlog.md` を参照する。

## 触るファイル

- 次の作業対象は追加依頼に応じて決める。現時点でREADMEや公開用ツール本体への未完了の編集はない。
- 新規候補を追加する場合は `docs/` の適切なカテゴリーを使う。公開済み資料の同期先は `PUBLICATION_MAP.md` を参照する。

## 注意

- README 更新時と翻訳文書の継続方針は `DECISIONS.md` を参照する。
- 翻訳対象は README.en.md の直接のリンク先、追加依頼の `USER_GUIDE.md` 8件、および `work-templates` の英語版テンプレート4件。各ガイド内で参照する補助文書や版別の `USER_GUIDE-<version>.md` までは再帰的に翻訳していない。
- `tools/whiteboard/app-generation-workflow/USER_GUIDE.en.md` は同じ場所の日本語原文（0.1.0）に対応する。別ディレクトリーの `workflows/app-generation-workflow/app-generation-workflow/USER_GUIDE.en.md`（0.2.3）とは版が異なる。
- `work-templates` の軽量な構成と英語版の配置・コピー時の命名方針は `DECISIONS.md` を参照する。日英 README のコピー例では、`WORK_TEMPLATES_DIR` を実際の `work-templates` ディレクトリーのパスへ置き換える。

## 関連ファイル

- `workflows/work-templates/NEXT_HISTORY.md`: 正本から取り込んだテンプレートの改善履歴。文中の Git コミット番号は正本側のリポジトリーを指す。
- `workflows/work-templates/README.md` / `workflows/work-templates/README.en.md`: 正本・コピーの位置付けと汎用コピー手順。
- `tools/anker-layout/README.md` / `tools/anker-layout/README.en.md`: 機能、通常・単体ビルド、サンプル、元配布物と公開用補足の案内。
- `tools/anker-layout/iccp/README.md` / `tools/anker-layout/iccp/README.en.md`: Qt Cling／icppでの対話操作例。`iccp/` は元配布物のディレクトリー名を保持している。

- `README.md` / `README.en.md`: リポジトリー全体の案内とツール一覧。
- 各ツールの `README.en.md` / `USER_GUIDE.en.md`、`tools/cling/README.en.txt`: 日本語文書に対応する英語版。
- `tools/qmlprofileranalyzer/USER_GUIDE.md` / `tools/qmlprofileranalyzer/USER_GUIDE.en.md`: QML Profiler トレース解析・比較・形式変換の利用方法。
- `DECISIONS.md`: 継続する判断、仕様、運用制約。
- `backlog.md`: 長期の残作業、問題点、保留事項。
