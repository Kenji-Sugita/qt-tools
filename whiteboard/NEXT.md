# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 次にやること

- 2026-09-18 にページ上限変更の検討用プロンプト草案を作成し、コミット `1e68fb169` に保存した。
  正本は `whiteboard-app/prompts/page_limit_change_prompt_draft.md`。
  現行仕様・変更案、未決事項D01〜D10、複数の回答例と得失評価、受け入れ条件案T01〜T14を記載した。
  利用者から回答が難しいため一旦コミットする指示を受け、判断は保留中。全案が未承認で、
  ページ上限変更も保存保護も未実装。現行の20ページ制限と正式仕様は変更していない。
  再開時は草案と `DECISIONS.md` の未決論点を確認し、利用者が再開を希望する項目から
  判断を支援する。回答例の推奨を採用済みと解釈せず、仕様確定と実装指示を得るまで実装しない。
  保留課題は `backlog.md` に記録した。今回の引き継ぎ更新は文書のみで、ビルド・実機試験は行わない。

- 2026-09-18 に描画データのインポート／エキスポートを実装した（未リリース）。
  File の専用メニューから `.whiteboard` の全ページ／現在ページを書き出し、
  `.whiteboard` と旧自動保存JSONを現在ページの直後へ取り込む。
  新規ID・コネクター参照変換、1回のUndo/Redo、上限・ロック・入力検証に対応する。
  ビルドとオフスクリーンの全16テストが成功。仕様書、日本語翻訳、利用ガイドMarkdown/PDFを更新した。
  ユーザーから実アプリでの動作確認完了の報告を受けた。利用ガイドPDFは21ページで描画確認済み。
  版番号は1.2.0のまま、CHANGELOGはUnreleasedに記録した。
  既存の配布ZIP・DMGとSites配布先には今回の機能をまだ反映していない。

- 本来の開発対象は `whiteboard-app/`。`whiteboard-app/whiteboard-app-qtmcpserver/` は
  他の方のバージョンを再現するための別環境であり、開発対象や起動アプリを混同しない。
- 2026-09-18 に本来の版へ `whiteboard/diagram/apply` の自動承認を追加した。
  `append_current`、`replace_current`、`new_page` が対象で、ほかの変更操作の承認は維持する。
  `WhiteboardApp`、`WhiteboardMcpTest`、`WhiteboardMcpHttpTest` のビルドと、
  `QT_QPA_PLATFORM=offscreen ctest --test-dir whiteboard-app/build -R '^WhiteboardMcp(Test|HttpTest)$' --output-on-failure`
  が成功（2 テスト）。実アプリで赤い矩形を要求し、ユーザーから承認なしで描画できたとの確認を得た。
  この実行時はツール側で `Unexpected response type` が返ったため、描画の成功はユーザー確認に基づく。
  次の実アプリ試験では応答エラーの再現有無も確認し、エラー時は重複描画を避けるため反映状態を先に確認する。
- 2026-09-18 にバージョンを 1.2.0 へ更新し、利用ガイド（Markdown/PDF）、チートシート
  （SVG/PNG）、CHANGELOG を同期した。CMake 再構成、アプリと MCP テストのビルド、
  MCP 関連 2 テストが成功し、コンパイル定義と MCP 応答の版番号を確認した。
  ガイド PDF は 20 ページで、版番号と変更箇所の描画を確認済み。
- 現時点でユーザーから指定された未完了実装はない。
- MCP 基盤拡張はコミット `733f6c520` で一区切りとする。次回は新しい tool の実装から始めず、
  起動済みの実アプリへ Codex から接続して、次の AI 編集シナリオ試験を順に行う。
  1. `whiteboard/elements/apply` で既存図の追加・更新・削除を一括実行し、承認表示、入力用
     `clientId` と永続 ID の対応、1 回の Undo で全体が戻ることを確認する。
  2. 要素一覧を取得した後に GUI で図を変更し、古い `expectedRevision` を指定した更新・削除・
     統合変更が、部分反映せず確実に拒否されることを確認する。
  3. 「図の作成、PNG 取得、配置または文言の部分修正、再取得」という AI の修正サイクルを行い、
     現在の 16 tool だけで無理なく完結するかを確認する。
  4. 試験結果から、整列・均等配置、重なり順、グループ化、ページ複製、テキスト検索、要素選択、
     自動レイアウトのうち、`whiteboard/elements/list` と `whiteboard/elements/apply` の組み合わせでは
     繰り返し複雑になる操作だけを、次段階の専用 tool 候補として整理する。
  専用 tool の追加は試験前に決めず、複数の実利用シナリオで同じ不便が再現することを採否判断の
  基準とする。試験後に結果と不足機能を提示し、実装範囲をユーザーと決める。
- 図形に追従するコネクターを実装済み。既存の直線要素に
  `startConnectionElementId` / `endConnectionElementId` を持たせ、矩形、角丸矩形、楕円、円、
  テキスト、画像への接続を表現する。Line ツールで端点を接続対象上に置いて作成すると
  接続し、接続先の移動、サイズ変更、回転、スケール、テキストサイズ変更、角丸変更に追従する。
  端点を手動移動して図形上で離した場合は操作対象端点をその図形へ再接続し、図形外で離した
  場合は自由端点のままにする。直線リサイズでは操作対象端点の接続を解除し、接続先削除時は
  線を残して失効した接続だけ解除する。コピー、貼り付け、MCP 図反映では接続先 ID を新しい
  永続 ID へ張り替える。
- 全描画要素への永続 ID 付与を実装済み。保存 JSON の各 `DrawingElement` に `id` を持たせ、
  `BoardDocument` が `nextElementId` を保存して `element-N` 形式で採番する。既存保存ファイルで
  ID がない要素や重複 ID は読み込み時に補完し、見た目、座標、重なり順は変更しない。
  新規作成、コピー/貼り付け、MCP 図反映で追加される要素には新しい ID を割り当てる。
  SVG 書き出しも保存 JSON と共通の永続 ID を外側 `g` の `id` に使う。Undo/Redo では
  `nextElementId` の到達値を維持し、取り消した作成操作の ID を後続の別要素へ再利用しない。
- 画像取り込み機能を実装済み。初期範囲は File の `Import Image...` から PNG、JPEG、BMP、SVG を
  取り込み、OS クリップボード画像を Paste で貼り付け、対応画像ファイルをキャンバスへ
  ドラッグ＆ドロップできる。SVG は PNG 化、保存 JSON は PNG Base64 埋め込み、画像要素は
  通常オブジェクトとして選択、移動、サイズ変更、回転、コピー、ペースト、グループ化、
  削除に対応する。
- WhiteboardApp のバージョンは `whiteboard-app/VERSION` の `1.2.0` を正本とし、CMake project version、
  アプリケーションバージョン、MCP `serverInfo.version`、利用ガイドをこの値に合わせる。
  リリース内容は `whiteboard-app/CHANGELOG.md` に記録する。
- 2026-08-30 の直近会話では、Apple Store 向け需要、価格、販売見込みを検討した。Whiteboard 単体は
  初期価格 US$4.99、4 アプリ構成（Whiteboard、VU メーター、レーザーポインター、セミナー用タイマー）
  は平均単価 US$5.99 前後を仮定。Small Business Program の 15% 手数料、USD/JPY 約 159.45 円換算では、
  4 アプリ構成の初年度 1,000-3,000 本で手取り約 81 万-244 万円、3-5 年総計 3,000-8,000 本で
  手取り約 244 万-650 万円を中心レンジと見た。これは市場確認用の粗い試算で、実装や価格仕様の
  決定ではない。
- 同じ検討では、Whiteboard 単体よりも VU メーター、レーザーポインター、セミナー用タイマーを
  含めた「発表・配信・授業支援ツール群」として見せる方が販売面で強いと整理した。初期方針は
  低価格の買い切り、各アプリの単体販売、後続のバンドルまたは Pro 版、App Store 説明文・
  スクリーンショット・短い動画の整備、日本語と英語の両対応を重視する。
- 現在ページの画像保存を実装済み。File の `Save Image...` から PNG/SVG と透明/白背景を選び、
  未選択時は全描画要素、選択時は選択要素だけを欠けない範囲へ切り抜いて保存する。
  SVG は通常要素をベクターで出力し、取り込み画像だけを Base64 data URI の `<image>` として
  埋め込む。各描画要素へ読みやすい一意な ID を付ける。
- `whiteboard-app/whiteboard-cheatsheet.svg` と PNG を 1.2.0 向けに更新済み。既存の6カード構成を
  維持し、回転、コネクター端点の接続変更、画像取り込み、PNG/SVG 保存、Undo/Redo、版番号を
  反映した。PNG は 1920 x 1240 で、SVG から `rsvg-convert` を使って生成する。
- WhiteboardApp の MCP Server 化を実装済み。通常起動では
  `http://127.0.0.1:8765/mcp` に Streamable HTTP Server を公開し、`--mcp` は互換用の
  標準入出力 Server として維持する。Whiteboard 固有 16 tool で状態・要素一覧、永続 ID による
  要素の一括更新・削除・追加更新削除の統合変更、ページ一覧・移動・管理、図反映、PNG/SVG の取得・保存、Undo/Redo に対応し、
  `whiteboard/diagram/apply` は自動承認し、それ以外の変更とファイル保存は GUI 承認を必須とする。
  要素更新・削除は全要求を検証してから一括反映し、
  1 呼び出しを 1 Undo とする。文書全体の単調増加 `revision` と `expectedRevision` で競合を拒否し、
  `diagram/apply` と統合変更は入力用 ID から採番済み永続 ID への対応を応答する。
- MCP は外部 qtmcpserver に依存しない WhiteboardApp 専用の最小 JSON-RPC 実装へ置き換え済み。
  `WhiteboardMcpController` が `initialize`、`tools/list`、`tools/call`、stdio 行区切り処理を担当し、
  `WhiteboardMcpHttpServer` が loopback Streamable HTTP、session ID、protocol version、Origin 制限を担当する。
- Codex の `~/.codex/config.toml` は `url = "http://127.0.0.1:8765/mcp"` に設定済み。
  接続時は WhiteboardApp を通常起動してから Codex を起動または MCP 接続を再読み込みする。
- `/mcp` の GET は SSE ストリームを提供しないため `405 Method Not Allowed` が正常応答。
  MCP の疎通確認は `Content-Type: application/json` と
  `Accept: application/json, text/event-stream` を付けた `initialize` の POST で行い、
  `HTTP/1.1 200 OK` と `Mcp-Session-Id` ヘッダーを確認する。
  Codex が `codex-visuals/*.html` や `visualize` を使った場合は WhiteboardApp MCP へ届いていないため、
  「WhiteboardApp の MCP ツールを使う。HTML や visualize は使わない」と明示して再実行する。
- 2026-07-31 に設定済みの HTTP MCP 接続から実アプリを操作し、`whiteboard/state` の取得と
  `whiteboard/diagram/apply` の `append_current` で 499 要素を一括反映できることを確認した。
  ユーザー保存ファイルは全 13 ページで、現在ページは 13 ページ目、同ページは 499 要素の
  ひまわり畑になっている。
- `/usr/local/qt/qtmcpserver` の `CustomOnly`、application-defined transport API、POSIX 標準入力
  通知/EOF 修正、macOS 対応単体テストは、別 Git リポジトリの `bf094f5` にコミット済み。
  現在の WhiteboardApp はこの外部リポジトリへ依存せず、上記は過去の参照記録である。
- macOS 配布 DMG は `qt/tools/utils/create-dmg.sh` で生成する。Finder のレイアウト処理は
  `/Volumes` へのランダムマウント、`tell disk`、`item ... of container window`、最大 10 回の
  アイコン配置リトライを使う。2026-08-06 に Codex から同スクリプトを実行して正式な
  `whiteboard-app/build/WhiteboardApp.dmg` を正常生成した。サイズは 43,435,029 bytes、
  SHA-256 は `af812a2a7f7a308fca70a40a2f9c588b53275c7f8b50be316b7f2d06644c2b39`。
  iTerm からの実行で Finder `-10006` が再現する場合は、Qt や DMG 内容ではなく Finder の
  アイコン表示状態と書き込みタイミングの差が有力。必要なら同じ生成コマンドの実行を Codex に依頼する。
- 長期課題と保留事項は `backlog.md`、採用済み仕様、不採用理由、運用制約は `DECISIONS.md` を参照する。
- 必要に応じて、リポジトリ直下のローカル確認用 ZIP `whiteboard-app.zip` の内容を確認する。
  トップは `whiteboard-app/`。この ZIP は生成物として Git 管理しない。
- 正式な source release ZIP は `icpp` と同様に
  `whiteboard-app/release/whiteboard-app-<version>-source.zip` へ作成し、Git 管理対象とする。
  バージョン `1.0.0` の場合は `whiteboard-app/release/whiteboard-app-1.0.0-source.zip`、
  ZIP 内トップ階層は `whiteboard-app-1.0.0/` とする。
- `whiteboard-app/release/whiteboard-app-1.0.0-source.zip` を作成済み。ZIP 内トップ階層は
  `whiteboard-app-1.0.0/`、111 files、展開後合計 2,593,565 bytes、ZIP サイズ 2.3 MB。
  SHA-256 は `46e52e0adcdffc4ed81bbfd6b2633ad9790ddef967dfc06a5e88c377a9f21d45`。
  `DECISIONS.md`、`NEXT.md`、`backlog.md`、`docs/`、`release/` は除外対象とする。
- `whiteboard-app/release/whiteboard-app-1.1.0-source.zip` を作成済み。ZIP 内トップ階層は
  `whiteboard-app-1.1.0/`、106 files、展開後合計 2,823,557 bytes、ZIP サイズ 1,664,694 bytes。
  SHA-256 は `c6cd2a79fa0f05c4aa59bb568ed8525ac86b1f275a1581fba8e515f59fc5fbb6`。
  除外対象 0 件、収録した `VERSION` は `1.1.0`、`unzip -t` は成功した。
- `whiteboard-app/release/whiteboard-app-1.2.0-source.zip` を作成済み。トップ階層は
  `whiteboard-app-1.2.0/`、106 files、ZIP サイズ 1,669,198 bytes。
  SHA-256 は `d058de9f34aa1380820103a31ea009db2aa6013c6d0f7dcc0845429931956879`。
  収録版 1.2.0、作業ツリーとの内容一致、除外対象 0 件、`unzip -t` 成功を確認した。
- 配布先 `/Users/sugita/Sites/tools/whiteboard/` の `USER_GUIDE.md`、`USER_GUIDE.pdf`、
  `whiteboard-app-1.2.0-source.zip` を更新済み。既存アイコンと旧版 ZIP は保持する。
  `index.html` の ZIP リンク・説明を 1.2.0 に更新し、埋め込みガイドを正本と同期した。
  コピー元とのバイト一致、埋め込み Markdown の一致、JavaScript 構文チェックが成功した。
  チートシート PNG/SVG は利用者の指示により配布先へコピーしない。今回追加した 2 ファイルは
  配布先から取り除き済み。ソース側のチートシートと ZIP 内の既存収録方針は維持する。
- 直近作成した `whiteboard-app.zip` は 2026-08-06 07:23 時点で 4,222,696 bytes、107 エントリ、
  展開後合計 5,147,463 bytes。SHA-256 は
  `bbaf0bb28e434076b9eef88c3ec736fdb2e75a2cc58bc011eb0e2687f838aa56`。
- 配布 ZIP からは `DECISIONS.md`, `NEXT.md`, `backlog.md`, `build/`, `release/`, `.DS_Store`,
  `docs/`, 仕様三点セット、プロンプト、要求メモ、一時画像/一時ファイル類を除外済み。
- WhiteboardApp 利用ガイドは `whiteboard-app/USER_GUIDE.md` と同ディレクトリの PDF を正本とし、
  リポジトリ直下にあった旧 Markdown/PDF と `whiteboard-app/` 内の重複 ZIP は削除済み。
  `app-generation-workflow/USER_GUIDE.md` と PDF は別用途のワークフローガイドとして維持する。
- オブジェクト回転機能を実装済み。選択枠外の回転ハンドルをドラッグし、Shift で 15 度単位にスナップする。Shift をドラッグ前またはドラッグ中に押した場合の両方に対応し、Shift 付きでハンドルを押しても選択解除されない。
- ページロック機能を実装済み。テンプレート用途の `UI Components / Wireframe Kit` と `Qt Designer Widget Kit` はユーザー保存ファイル上でも `locked: true` に更新済み。
- 2026-08-03 の画像保存と配布整理は、実装 `c469c11b7`、利用ガイド更新 `6087d3f58`、
  ガイド/ZIP 配置統一 `d7c708652`、ZIP 再生成記録 `e63865d36`、旧 PDF 削除 `12224d64d`。
  新規実装ファイルの作成開始から実装コミットまでは 21 分 58 秒で、事前調査と指示確認を含む
  実装作業は約 25-30 分。QtMcpServer 側は `bf094f5 Add application-defined MCP transport support`。
  DMG 関連は `051621ffd Harden Finder DMG layout handling` と
  `1bfe2dbf6 Fix Finder DMG icon positioning`。
- FAB の子ボタン順を位置に応じて切り替える変更を実装済み。「選択」は常に親 FAB の隣に配置し、子ボタンを開いたまま画面中央をまたいでドラッグした場合も展開方向と順番を更新する。
- ユーザー保存ファイルの 3 ページ目に `Qt QSizePolicy` 図解ページを追加済み。横・縦ポリシー、
  7 種類の比較、レイアウト例、サイズ制約を 4 区画に分けてグループ化している。後続ページ追加後の
  現在状態は全 13 ページで、13 ページ目を選択している。
- ツールバーの幅超過時は `PersistentToolBar` の独自拡張ボタンを表示する。クリックで拡張パネルを
  開閉し、マウス退出やパネル内操作では閉じない。同じ操作部品を通常行とパネル間で再配置し、
  パネル外のキャンバス入力を妨げない。
- 直近で大きいファイルのリファクタは一段落。`MainWindow.cpp`, `BoardModel.cpp`, `CanvasWidget.cpp` はまだ大きいが、共有状態が多いため現時点では追加分割しない方針。

## 未完了

- 現時点でユーザーから指定された未完了実装はない。
- ページ上限変更は検討保留。草案の作成・回答例の追記は完了しているが、仕様確定・実装の指示は未受領。
- フル再生成比較は直近作業ではないため、詳細を `backlog.md` へ移した。
- MCP の Windows/MSVC 確認と次回配布物の再生成は `backlog.md` に記録した。外部 qtmcpserver は不要。

## 触るファイル

- ページ上限変更の検討再開時: `whiteboard-app/prompts/page_limit_change_prompt_draft.md`。
  現行の `prompts/implementation_prompt.md` と三仕様書への反映は、未決事項の承認後に行う。
- 描画データ交換: `whiteboard-app/include/BoardDocumentExchange.h`、
  `whiteboard-app/src/BoardDocumentExchange.cpp`、`whiteboard-app/src/BoardModel.cpp`、
  `whiteboard-app/src/MainWindow.cpp`、`whiteboard-app/tests/tst_boarddocumentexchange.cpp`。
- macOS DMG 作成共通スクリプト: `../utils/create-dmg.sh`。Finder のアイコンは
  `container window` の項目として配置し、失敗時は各試行のエラー番号とメッセージを出力する。
- 直近の UI 実装: `whiteboard-app/src/MainWindow.cpp`, `whiteboard-app/include/MainWindow.h`,
  `whiteboard-app/src/FloatingActionButton.cpp`, `whiteboard-app/include/FloatingActionButton.h`,
  `whiteboard-app/src/FloatingActionChildButton.cpp`, `whiteboard-app/include/FloatingActionChildButton.h`,
  `whiteboard-app/src/FloatingActionBubbleLabel.cpp`, `whiteboard-app/include/FloatingActionBubbleLabel.h`,
  `whiteboard-app/src/CenteredComboBox.cpp`, `whiteboard-app/include/CenteredComboBox.h`。
- 直近の設定保存: `whiteboard-app/include/BoardSettings.h`, `whiteboard-app/src/BoardSettings.cpp`,
  `whiteboard-app/include/BoardModel.h`, `whiteboard-app/src/BoardModel.cpp`。
- 回転関連: `whiteboard-app/include/DrawingElement.h`, `whiteboard-app/src/DrawingElement.cpp`,
  `whiteboard-app/include/CanvasWidget.h`, `whiteboard-app/src/CanvasWidget.cpp`,
  `whiteboard-app/src/Page.cpp`, `whiteboard-app/tests/tst_boardmodel_selection.cpp`,
  `whiteboard-app/tests/tst_boardmodel_document.cpp`, `whiteboard-app/tests/tst_canvaswidget_selection.cpp`。
- 利用ガイドの正本は `whiteboard-app/USER_GUIDE.md`。PDF 版は
  `whiteboard-app/USER_GUIDE.pdf`。
- 直近のドキュメント: `whiteboard-app/USER_GUIDE.md`, `whiteboard-app/USER_GUIDE.pdf`,
  `whiteboard-app/source_memo.md`, `whiteboard-app/specs/02_ui_spec.md`,
  `whiteboard-app/prompts/implementation_prompt.md`, `whiteboard-app/implementation_design.md`。
- 直近の翻訳: `whiteboard-app/translations/whiteboard_ja.ts`。日本語以外はソース上の英語文字列を表示する。
- 画像保存関連: `whiteboard-app/include/ImageExporter.h`, `whiteboard-app/src/ImageExporter.cpp`,
  `whiteboard-app/include/DrawingRenderer.h`, `whiteboard-app/src/DrawingRenderer.cpp`,
  `whiteboard-app/include/ImageExportDialog.h`, `whiteboard-app/src/ImageExportDialog.cpp`,
  `whiteboard-app/tests/tst_imageexporter.cpp`, `whiteboard-app/tests/tst_imageexportdialog.cpp`。
- リポジトリ直下の `whiteboard-app.zip` はローカル確認用生成物。`whiteboard-app/` 内には
  同名 ZIP を作らない。この ZIP は必要なら作成・確認するが Git 管理対象にはしない。
- 正式な source release ZIP は `whiteboard-app/release/whiteboard-app-<version>-source.zip`。
  これは release 成果物として Git 管理する。
- テストは分類済み。`BoardModel` は `tst_boardmodel_drawing.cpp`, `tst_boardmodel_style.cpp`, `tst_boardmodel_curve.cpp`, `tst_boardmodel_selection.cpp`, `tst_boardmodel_document.cpp`。`CanvasWidget` は `tst_canvaswidget_curve.cpp`, `tst_canvaswidget_shape_text.cpp`, `tst_canvaswidget_selection.cpp`。
- `FloatingActionButton` のクリック/ドラッグ判定テストは `whiteboard-app/tests/tst_floatingactionbutton.cpp`。
- ツールバー拡張の実装は `whiteboard-app/include/PersistentToolBar.h`,
  `whiteboard-app/src/PersistentToolBar.cpp`。macOS では管理対象ウィジェットへ
  `Qt::WA_LayoutUsesWidgetRect` を設定して Qt のレイアウト項目矩形と独自計算の矩形を一致させる。
  行分割だけを項目幅と間隔の累積で判断し、外側の
  `QVBoxLayout` とスペーサーなしの行別 `QHBoxLayout` で配置する。最終幅は外側レイアウトの
  `sizeHint()` とフレーム幅から決めて暗黙の余剰空間を作らない。
  右端背景が長く見えた直接原因は、視覚上の右端だった Lock の後ろに透明なページ番号ラベルが
  末尾項目として存在していたこと。ページ操作はページ番号、Previous、Next、Add、Delete、Lock の
  順に変更し、Lock を実際の末尾項目にした。
  環境依存制約の正本は
  `whiteboard-app/implementation_constraints.md`、テストは `whiteboard-app/tests/tst_persistenttoolbar.cpp`。
- MCP 関連: `whiteboard-app/include/WhiteboardMcpController.h`、
  `whiteboard-app/src/WhiteboardMcpController.cpp`、`whiteboard-app/include/WhiteboardMcpHttpServer.h`、
  `whiteboard-app/src/WhiteboardMcpHttpServer.cpp`、`whiteboard-app/tests/tst_whiteboard_mcp.cpp`、
  `whiteboard-app/tests/tst_whiteboard_mcp_http.cpp`、
  `docs/mcp_diagram_capabilities.md`。
- バージョン・リリース管理: `whiteboard-app/VERSION`、`whiteboard-app/CHANGELOG.md`、
  `whiteboard-app/CMakeLists.txt`。
- 画像取り込み関連: `whiteboard-app/include/ImageImporter.h`, `whiteboard-app/src/ImageImporter.cpp`,
  `whiteboard-app/include/CanvasWidget.h`, `whiteboard-app/src/CanvasWidget.cpp`,
  `whiteboard-app/include/MainWindow.h`, `whiteboard-app/src/MainWindow.cpp`,
  `whiteboard-app/tests/tst_imageimporter.cpp`, `whiteboard-app/tests/tst_canvaswidget_selection.cpp`。
  画像要素は `DrawingElement` に `imageData` と `imageMimeType` を持たせ、`DrawingRenderer` と
  `ImageExporter` で描画・書き出しする。
- 永続 ID 関連: `whiteboard-app/include/DrawingElement.h`, `whiteboard-app/src/DrawingElement.cpp`,
  `whiteboard-app/include/BoardDocument.h`, `whiteboard-app/src/BoardDocument.cpp`,
  `whiteboard-app/include/BoardModel.h`, `whiteboard-app/src/BoardModel.cpp`,
  `whiteboard-app/src/ImageExporter.cpp`, `whiteboard-app/tests/tst_boardmodel_document.cpp`,
  `whiteboard-app/tests/tst_imageexporter.cpp`。
- コネクター関連: `whiteboard-app/include/DrawingElement.h`, `whiteboard-app/src/DrawingElement.cpp`,
  `whiteboard-app/include/BoardModel.h`, `whiteboard-app/src/BoardModel.cpp`,
  `whiteboard-app/src/CanvasWidget.cpp`,
  `whiteboard-app/src/WhiteboardMcpController.cpp`,
  `whiteboard-app/tests/tst_boardmodel_selection.cpp`,
  `whiteboard-app/tests/tst_boardmodel_document.cpp`。
- 実装設計契約: `whiteboard-app/implementation_design.md` は責務別テスト構成、MCP HTTP、FAB、
  ツールバー UI、ページ上限 20 件へ更新済み。`whiteboard-app/prompts/implementation_prompt.md` も
  同じ方針に従うよう更新済み。
- ページロック関連: `whiteboard-app/include/Page.h`, `whiteboard-app/src/Page.cpp`, `whiteboard-app/include/BoardModel.h`, `whiteboard-app/src/BoardModel.cpp`, `whiteboard-app/include/MainWindow.h`, `whiteboard-app/src/MainWindow.cpp`, `whiteboard-app/src/CanvasWidget.cpp`, `whiteboard-app/assets/icons/page-lock.svg`, `whiteboard-app/assets/icons/page-unlock.svg`。
- `Qt QSizePolicy` 図解ページは Git 管理外の `/Users/sugita/Library/Application Support/WhiteboardApp/whiteboard.json` に保存している。追加ページは未ロックで、区画ごとのグループ ID は 27 から 30。

## 注意

- 恒久的な仕様、設計判断、不採用理由、配布制約は `DECISIONS.md` を正本とする。
- 長期の調査課題と保留事項は `backlog.md` を正本とする。
- 直近検証: 2026-07-27 に `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build whiteboard-app/build` と `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 ctest --test-dir whiteboard-app/build --output-on-failure` が成功。回転と Shift 操作を含む 9 テストターゲットはすべて成功。実行ファイルは `arm64` / `x86_64` universal binary。配布 ZIP は `unzip -t` で異常なし。
- MCP 直近検証: 2026-07-31 に Whiteboard 11 テストターゲットと QtMcpServer 6 テストターゲットが
  すべて成功。通常起動した実 WhiteboardApp の HTTP endpoint で initialize と session ID の
  応答を確認し、Codex 設定が `streamable_http` と認識されることを確認した。さらに
  `whiteboard/state` と 499 要素の `whiteboard/diagram/apply` が実アプリで成功した。
  `macdeployqt` の試験用コピーには `libqtmcpserver.1.dylib`、Qt HttpServer、Qt Network が
  収録され、アプリと依存 framework の universal binary を確認した。
  現在の WhiteboardApp は外部 qtmcpserver ライブラリを使わないため、次回配布では
  `libqtmcpserver.1.dylib` が含まれないことを確認する。
- MCP ツール拡張の直近検証: 2026-08-03 に全ターゲットをビルドし、ページ移動・管理、
  PNG/SVG の取得・保存、Redo、状態・ページ一覧を含む Whiteboard 全 13 テストが成功した。
- MCP 独自実装の直近検証: 2026-09-02 に `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build whiteboard-app/build` が成功した。
  `QT_QPA_PLATFORM=offscreen` 付きの通常 sandbox では HTTP loopback 待受だけが制限で失敗し、
  sandbox 外で `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 QT_QPA_PLATFORM=offscreen ctest --test-dir whiteboard-app/build --output-on-failure`
  を実行して Whiteboard 全 14 テストが成功した。
- バージョン・リリース管理の直近検証: 2026-09-02 に `whiteboard-app/VERSION` を `1.0.0` とし、
  CMake configure、ビルド、MCP `serverInfo.version` 確認を含む Whiteboard 全 14 テストが成功した。
  `USER_GUIDE.pdf` は 18 ページで再生成し、表紙と版管理説明ページを PNG レンダリングで確認した。
- バージョン 1.1.0 の直近検証: 2026-09-06 に `whiteboard-app/VERSION` と利用ガイドを `1.1.0` へ
  更新し、CMake configure、ビルド、MCP `serverInfo.version` 確認を含む Whiteboard 全 15 テストが
  成功した。`USER_GUIDE.pdf` は front matter の `genpdf.format: book` を使って 20 ページで
  再生成し、表紙、ページ番号、著作権表記と全ページの描画を確認した。
- 画像取り込み機能の直近検証: 2026-09-05 に
  `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build whiteboard-app/build` が成功し、
  `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 QT_QPA_PLATFORM=offscreen ctest --test-dir whiteboard-app/build --output-on-failure`
  で Whiteboard 全 15 テストが成功した。追加テストでは PNG/JPEG/BMP/SVG 取り込み、QImage 入力、
  SVG の PNG 化、画像要素の保存読み込み、画像ファイルのドラッグ＆ドロップ、ロック中のドロップ拒否、
  SVG 書き出し時の `<image>` 埋め込みを確認した。
- 永続 ID の直近検証: 2026-09-06 に
  `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 cmake --build whiteboard-app/build` が成功し、
  `QTFRAMEWORK_BYPASS_LICENSE_CHECK=1 QT_QPA_PLATFORM=offscreen ctest --test-dir whiteboard-app/build --output-on-failure`
  で Whiteboard 全 15 テストが成功した。追加テストでは新規作成、コピー/貼り付け、
  MCP 図反映、旧保存ファイル補完、SVG 書き出しで描画要素 ID が一意になることを確認した。
- コネクターの直近検証: 2026-09-06 に
  `cmake --build whiteboard-app/build` と
  `ctest --test-dir whiteboard-app/build --output-on-failure` が成功し、Whiteboard 全 15 テストが
  成功した。追加テストでは接続済み直線が接続先図形の移動に追従すること、手動端点移動で
  図形外なら接続解除され、図形上で離すと再接続されること、コピー/貼り付けと MCP 図反映相当の
  一括追加で接続先 ID が新しい永続 ID へ張り替わることを確認した。
- source release ZIP の直近検証: 2026-09-02 に
  `whiteboard-app/release/whiteboard-app-1.0.0-source.zip` を `docs/` 除外で作り直し、
  `unzip -l` で `whiteboard-app-1.0.0/` トップ階層と 111 files を確認した。
  `DECISIONS.md`、`NEXT.md`、`backlog.md`、`docs/`、`release/` を含む除外対象 0 件、
  `unzip -t` 成功。
- source release ZIP 1.1.0 の直近検証: 2026-09-06 に
  `whiteboard-app/release/whiteboard-app-1.1.0-source.zip` を作成し、106 files、トップ階層
  `whiteboard-app-1.1.0/`、収録版 `1.1.0`、除外対象 0 件、`unzip -t` 成功を確認した。
- 画像保存実装の直近検証: 2026-08-03 に CMake configure、アプリのビルド、画像保存の
  PNG/SVG・設定ダイアログテストを含む Whiteboard 13 テストターゲットがすべて成功した。
- 配布 ZIP の直近検証: 2026-08-06 に Git 管理対象の期待ファイル一覧との完全一致、必須ファイル、
  除外対象 0 件、トップ階層 `whiteboard-app/`、`unzip -t` の成功を確認した。
- 利用ガイドの直近検証: 2026-08-06 にツールバー拡張パネルの開閉、表示維持、自動折りたたみ、
  ページ操作順を反映した 18 ページの PDF を再生成し、全ページの描画、表、コード、改ページを確認した。
- ツールバー拡張の直近検証: 2026-08-05 に `PersistentToolBar` の専用テストを追加し、クリック開閉、
  マウス退出と拡張項目操作後の表示維持、外側 `QToolBar` での幅使用、通常行と拡張行の操作部品の
  左端揃え、拡張パネル背景と外側 `QToolBar` の左端揃え、ページ番号ラベルを先頭、Lock を末尾とする
  配置後の先頭・末尾ウィジェット座標を基準とする左右背景余白の一致、固定幅を超える
  `sizeHint()` の除外、macOS の `Qt::WA_LayoutUsesWidgetRect` による矩形正規化、リサイズ時の
  拡張解除、折り返した各行の左端一致を確認した。専用テスト 10 件は
  オフスクリーン環境と Cocoa ネイティブ環境の両方で成功した。アプリを含む全ターゲットのビルドと
  Whiteboard 全 14 テストが
  成功した。HTTP MCP テストは loopback 待受を許可した環境で再実行して成功した。
- DMG 作成の直近検証: 2026-08-06 にビルド済み `WhiteboardApp.app` から Finder レイアウト付きの
  正式な `WhiteboardApp.dmg` を生成した。`hdiutil convert`、最終ファイル作成、`hdiutil verify` が
  成功し、作業用 `WhiteboardApp-rw.dmg` が残っていないことを確認した。iTerm で発生した
  Finder `-10006` は、同じスクリプトを Codex から実行した際には再現しなかった。
- ユーザー保存ファイル `/Users/sugita/Library/Application Support/WhiteboardApp/whiteboard.json` は、選択不能切り分けのため `selectedTool` を `select` に戻した。
- `WhiteBoardApp-Win.zip`、`WhiteboardApp.dmg`、`whiteboard-and-laserpointer.png`、
  `whiteboard-app/build/`、`tmp/` は未追跡生成物。コミット対象外。
