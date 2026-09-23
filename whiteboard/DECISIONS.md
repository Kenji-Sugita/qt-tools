# DECISIONS

## 採用済み

- 編集可能な描画データの交換は `.whiteboard`（`WhiteboardApp.DrawingData`、形式バージョン1のJSON）を使う。
  全ページ／現在ページを書き出し、取り込みは現在ページの直後へのページ追加とする。
  旧自動保存JSONも検証後に受理する。要素ID・コネクター参照を張り替え、1回のUndo/Redoとする。
  現在ページと操作設定は維持し、20ページ超過・不正データは全体拒否する。
  ロック中からの取り込みは禁止し、空ページ・ロック中の書き出しは許可する。
  全文書置換、選択要素だけの書き出し、MCP tool追加は今回の対象外とする。

- ページ追加は現在ページの直後へ挿入する。最大ページ数は
  `BoardDocument::MaxPages = 20` を唯一の基準とし、保存復元時も同じ上限を使う。
- ページロック中は選択、コピー、ページ移動、ズーム、スクロールを許可する。描画追加、削除、
  貼り付け、移動、リサイズ、頂点編集、テキスト編集、スタイル変更、グループ化、Undo/Redo、
  ページ削除は禁止する。状態は保存 JSON の `locked` で保持する。
- 回転は選択枠外のハンドルで操作する。単一選択はオブジェクト中心、複数選択とグループは
  選択範囲中心を基準にし、Shift で 15 度単位にスナップする。Shift はドラッグ前とドラッグ中の
  どちらで押しても有効にする。単一オブジェクトはハンドルのダブルクリックで 0 度へ戻す。
  見た目が変わらない単体の円には回転ハンドルを表示しない。
- 回転角は保存 JSON の `rotationDegrees` で保持する。キーがない既存ファイルは 0 度として
  読み込み、旧保存ファイルから新アプリへの後方互換性を維持する。新アプリの回転済みファイルを
  旧アプリで開く前方互換性は保証しない。
- 描画要素の `element-N` 永続 ID は Undo/Redo で `nextElementId` を巻き戻さず、同じ文書を
  編集している間に一度発行した ID を別要素へ再利用しない。MCP クライアントが保持している
  削除済み要素の ID で、後から作成した別要素を誤操作することを防ぐ。
- FAB は右端に浮く実領域パネルとし、見えない領域でキャンバス入力を奪わない。子ボタンは
  `geometry` と `opacity` を使ってアニメーションし、展開方向にかかわらず「選択」を親 FAB の
  隣に置く。開いたまま親 FAB を画面中央越しに移動した場合も方向と順序を更新する。
- FAB 子ボタンの操作名は専用の `FloatingActionBubbleLabel` で左側へ表示する。標準ツールチップや
  hover 拡大は使わず、ラベル幅は左側の空き幅と `sizeHint()` で決める。
- ツールバーの幅超過時は `PersistentToolBar` の独自拡張ボタンとパネルを使う。クリックで開閉し、
  マウス退出やパネル内操作では閉じない。操作部品は通常行とパネルの間で同じ実体を再配置し、
  カテゴリ別背景色とチャコールグレー背景を維持する。拡張行はスペーサーなしの `QHBoxLayout`、
  行全体は `QVBoxLayout` で構成し、項目幅の累積は折り返し判定だけに使う。最終幅は配置後の
  レイアウトの `sizeHint()` から決める。ページ番号ラベルはページ操作の先頭、Lock はツールバーの
  末尾に置き、透明なラベル領域が視覚上の右余白として残らないようにする。
- 閉じたコンボボックスの文字揃えには専用の `CenteredComboBox` を使う。
- UI の基準文字列は英語とし、日本語環境では `QTranslator` と
  `:/i18n/whiteboard_ja.qm` を使う。CMake は `LinguistTools` と
  `qt_add_translations()` を使う。
- WhiteboardApp のバージョンは `whiteboard-app/VERSION` を正本とする。CMake project version、
  アプリケーションバージョン、MCP `serverInfo.version` は同じ値へ合わせ、リリース内容は
  `whiteboard-app/CHANGELOG.md` に記録する。
- macOS は `arm64;x86_64` の universal binary を既定とし、使用する Qt にも両アーキテクチャを
  要求する。
- `MainWindow.cpp`、`BoardModel.cpp`、`CanvasWidget.cpp` は共有状態が多いため、現時点では
  追加分割しない。リファクタでは既存仕様を優先し、テストは責務別のファイル分割で管理する。
- 機能、UI、実装制約の変更はコードだけで終わらせず、`source_memo.md`、必要な
  `specs/*.md`、`implementation_design.md`、`prompts/implementation_prompt.md` への反映要否を
  毎回確認する。
- リポジトリ直下の `whiteboard-app.zip` はローカル確認用の生成物で、Git 管理しない。
  正式な source release ZIP は `icpp` と同様に
  `whiteboard-app/release/whiteboard-app-<version>-source.zip` へ作成し、Git 管理対象とする。
  release ZIP のトップ階層は `whiteboard-app-<version>/` とする。収録範囲と除外対象は
  `NEXT.md` の配布情報および本書の運用制約に従う。
- WhiteboardApp の通常起動時は `127.0.0.1:8765/mcp` で Streamable HTTP MCP Server を公開し、
  MCP クライアントは URL で接続する。Codex 起動のたびに GUI が自動起動する運用を避けるため、
  クライアントからアプリを自動起動せず、利用者が先に WhiteboardApp を通常起動する。
- `--mcp` による標準入出力接続は、既存 MCP クライアントとの互換モードとして維持する。
- HTTP Server は loopback 接続と localhost 系 Origin だけを許可し、MCP セッション ID と
  protocol version を検証する。ポート競合時も GUI 本体は起動し、警告を表示する。
- Streamable HTTP endpoint `/mcp` は JSON-RPC の POST を対象とし、GET の SSE ストリームは
  提供しない。GET 確認で `405 Method Not Allowed` が返るのは正常とし、疎通確認は
  `initialize` の POST と `Mcp-Session-Id` ヘッダーの有無で行う。
- MCP は外部 qtmcpserver に依存しない WhiteboardApp 専用の最小 JSON-RPC 実装とする。
  公開 tool は状態・要素一覧、永続 ID による要素の一括更新・削除、ページ一覧、ページ移動・管理、
  統合要素変更、図の一括反映、現在ページ PNG、PNG/SVG の取得・保存、Undo/Redo の Whiteboard 固有 16 件に限定し、
  汎用 QObject/QEvent 操作は公開しない。
- MCP の読み取りとページ移動は承認不要とする。AI からの描画時の確認操作を省くため、
  `whiteboard/diagram/apply` は全モードで自動承認する。
  要素更新・削除・統合変更、ページ追加・削除・ロック、画像ファイル保存、Undo/Redo は
  Qt の確認ダイアログで毎回承認を得る。
  図反映は新規ページ、現在ページへの追加、現在ページの置換に対応し、1 呼び出しを 1 Undo とする。
- MCP 入力は要求全体を検証してから一括反映し、一部反映や暗黙補正を行わない。既存保存 JSON の
  形式は変更せず、成功した変更は通常の保存処理で保存する。
- 文書全体に単調増加する `revision` を保存し、MCP の要素更新・削除・統合変更は読み取り時の
  `expectedRevision` と一致する場合だけ実行する。GUI、Undo/Redo、別の MCP 操作による変更後に
  古い要素状態を使った要求が別要素を上書きすることを防ぐ。
- `whiteboard/elements/apply` は追加・更新・削除を合計 500 操作まで一括検証・一括反映し、
  1 呼び出しを 1 Undo とする。追加要素の `clientId` は新しい永続 ID へ変換して応答する。
- `whiteboard/diagram/apply` は 1 回につき 1 から 500 要素を受け付ける。`new_page` は図を伴う
  新規ページ作成用とし、要素のない空ページは既存 GUI のページ追加操作を使う。
- `whiteboard/image/save` は絶対パス、PNG/SVG と一致する拡張子、上書き可否の明示を要求する。
- 画像保存は現在ページだけを対象とし、未選択時は全描画要素、選択時は選択要素だけを出力する。
  PNG と SVG に対応し、同じ欠け防止範囲を使う。PNG はキャンバス座標 1 単位を 1px とし、
  SVG は通常要素をベクターで出力し、取り込み画像だけを Base64 data URI の `<image>` として
  埋め込む。SVG の各描画要素には保存 JSON と共通の永続 ID を付ける。
- 画像保存の背景は透明を既定とし白も選択可能にする。File の `Save Image...` から形式と背景を
  選ぶ設定ダイアログを表示し、その後に標準ファイルダイアログを使う。ツールバーへは追加しない。
  空ページでは無効、ページロック中は有効とし、保存失敗時は同じ条件で再試行できるようにする。
- 画像取り込みは File の `Import Image...`、OS クリップボード画像の貼り付け、キャンバスへの
  画像ファイルドラッグ＆ドロップから行い、PNG、JPEG、BMP、SVG に対応する。
  SVG はベクター分解せず PNG 化し、画像要素として保存 JSON に Base64 埋め込みする。
  ファイル選択と貼り付けは表示中範囲の中央、ドロップはドロップ位置を基準に配置する。
  画像要素は選択、移動、サイズ変更、回転、コピー、ペースト、グループ化、削除に対応し、
  線幅、線種、塗り潰し、矢印、フォント、スタイルリセットの対象外にする。
- 全描画要素には保存 JSON 上の永続 ID を付与する。通常は `element-N` 形式で `BoardDocument` が
  採番し、`nextElementId` を保存する。旧保存ファイルで ID がない要素や重複 ID は読み込み時に
  一意化し、図形の見た目、座標、重なり順は変更しない。コピー、貼り付け、MCP 図反映で
  新規作成される要素には入力元と異なる新しい ID を付ける。
- 図形に追従するコネクターは、専用要素型ではなく既存の `line` に
  `startConnectionElementId` / `endConnectionElementId` を持たせて表現する。接続対象は矩形、
  角丸矩形、楕円、円、テキスト、画像とし、端点は対象の境界へ正規化する。接続先の移動、
  サイズ変更、回転、スケール、テキストサイズ変更、角丸変更では `BoardModel` が端点を再計算する。
  端点を手動移動して図形上で離した場合は操作対象端点をその図形へ再接続し、図形外で離した場合は
  自由端点のままにする。直線リサイズ時は操作対象端点の接続を解除する。接続先削除時は線を残して
  失効した接続だけ解除する。コピー、貼り付け、MCP 図反映では接続先 ID を新しい永続 ID へ
  張り替える。

## 不採用

- `QToolBar` 内部の右端拡張ボタンや内部 `QMenu` を後追い制御する方式。クリック不能や
  キャンバス選択不能が発生したため採用しない。`PersistentToolBar` は Qt 内部を操作せず、
  自身が所有する操作部品と拡張パネルだけを制御する。
- FAB のためにフル高さ透明パネルや input mask を設ける方式。キャンバス右側の入力を奪うため
  採用しない。
- `CenteredComboBox` を editable 化したり、クリックイベントを横取りして中央揃えする方式。
- 仕様差分管理用の `spec_update_requests.md`。現行ワークフローと合わないため削除済みで、
  今後も通常の同期対象文書を直接更新する。

## まだ決めていないこと

- ページ上限変更は実装前にプロンプト草案を作り、未決事項を明確にしてから判断する。
  草案と回答例・得失評価は `whiteboard-app/prompts/page_limit_change_prompt_draft.md`
  （コミット `1e68fb169`）に保存済み。利用者は回答を一旦保留しており、推奨案も未承認。
  本書の採用済み20ページ制限を変更する決定ではなく、草案には正式仕様を上書きする効力はない。
  未決事項は上限の意味、設定範囲・性能、UI、永続化・不正値、Undo/Redo、最大規模の取り込み、
  旧版共存、MCPとrevision、通常保存保護の範囲、外部入力の資源制限の10件。
  保存保護を先行実装する提案も未承認であり、暗黙の前提作業として開始しない。
  再開時はデータ保持、既存動作との整合、説明の分かりやすさ、実装・検証範囲で選択肢を評価する。
  上限変更、既存ページ保持、履歴復元、ファイル解析時の安全制限を混同しない。
  決定後に正式仕様・実装プロンプト・受け入れ条件を整合させ、実装指示を得て着手する。

- フル再生成比較をいつ、どの範囲で再開するか。準備事項は `backlog.md` と
  `whiteboard-app/regeneration_alignment_plan.md` に記録する。

## 判断基準

- 操作性の変更では、キャンバス上の選択・クリック・ドラッグを阻害しないことを最優先する。
- 既存挙動を維持できないリファクタや、共有状態を不自然に分断するファイル分割は行わない。
- 保存形式を拡張する場合は、欠落キーに安全な既定値を設け、旧保存ファイルの読み込みをテストする。
- 共通操作や状態遷移へ影響する変更は、局所的な単体テストだけでなく関連するテストを通し、
  MCP、画像保存、ツールバー拡張を含む全体回帰では全テストターゲットを確認する。
- 配布物は内容一覧、展開階層、圧縮データ整合性、universal binary を確認してから渡す。
- 外部操作 API はアプリ固有 tool だけを公開し、QObject やイベント注入など内部実装を直接操作する
  汎用 tool を公開しない。

## 継続する仕様・運用制約

- 配布先 `~/Sites/tools/whiteboard/` へチートシート PNG/SVG を単独コピーしない。
  ソース側での保管・更新と source release ZIP の収録方針は従来どおりとする。

- 作業再開に直接必要な現在状態は `NEXT.md`、長期の残作業、問題点、保留事項は
  `backlog.md`、今後も有効な仕様、設計判断、不採用理由、未決論点、判断基準、運用制約は
  `DECISIONS.md` を正本とする。
- macOS DMG は `../utils/create-dmg.sh` で生成する。Finder レイアウトは `/Volumes` のランダムな
  マウント先、`tell disk`、`item ... of container window`、最大 10 回のアイコン配置リトライを使う。
  Finder のディスク項目へ直接 `position` を設定すると `-10006` になるため、必ず container window
  の項目として指定する。`macdeployqt` の `qt.conf already exists` / `Plugins = PlugIns` 警告は
  Finder `-10006` と無関係。同じスクリプトが別の実行環境では成功する場合の `-10006` は、Qt や
  アプリ内容の問題と断定せず、Finder のアイコン表示状態、Apple Event の処理タイミング、実行元の
  Automation 許可を切り分ける。配布作成を優先する場合は、正常動作を確認済みの Codex 実行を使える。
- source release ZIP には少なくとも `DECISIONS.md`、`NEXT.md`、`backlog.md`、
  `whiteboard-app/build/`、`whiteboard-app/release/`、`.DS_Store`、`docs/`、`specs/`、`prompts/`、
  `request.txt`、`requirement.txt`、`regeneration_alignment_plan.md`、`implementation_design.md`、
  `whiteboard-cheatsheet.svg`、`o`、`oo`、`o.png`、`whiteboard-and-laserpointer.png` を入れない。
- WhiteboardApp 利用ガイドの正本は `whiteboard-app/USER_GUIDE.md` とし、PDF 版も同じ
  ディレクトリへ置く。リポジトリ直下へ同じ利用ガイドを複製しない。
  `app-generation-workflow/USER_GUIDE.md` と PDF は別用途のワークフローガイドとして維持する。
- リポジトリ直下の `whiteboard-app.zip` は Git 管理外のローカル確認用生成物とし、
  正式な source release ZIP は `whiteboard-app/release/` にバージョン付きで置く。
  `whiteboard-app/` 内へ同名の `whiteboard-app.zip` は複製しない。
- ユーザー保存ファイルは
  `/Users/sugita/Library/Application Support/WhiteboardApp/whiteboard.json` にあり、Git 管理外。
- `whiteboard-app/build/`、`tmp/`、リポジトリ直下の `whiteboard-app.zip` は生成物であり、コミットしない。
