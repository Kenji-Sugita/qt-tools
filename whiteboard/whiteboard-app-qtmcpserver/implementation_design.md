# ホワイトボードアプリ 実装設計契約

このファイルは、仕様からコードを再生成するときに維持する実装設計の正本である。
機能仕様だけでは再生成時に別実装へ寄るため、現在の実装から維持すべき責務分担、
状態遷移、保存形式、UI 構成を固定する。

## 目的

- `source_memo.md` と `specs/*.md` は外部仕様を定義する。
- `implementation_constraints.md` は環境依存や横断的な実装制約を定義する。
- このファイルは、再生成時に既存実装と同じ構造・責務・操作状態を維持するための
  実装設計契約を定義する。
- ここにない便利機能や構造変更は追加しない。構造変更が必要な場合は、先にこの契約
  を更新する。

## ディレクトリとターゲット構成

- ソースは `include/` と `src/` に分ける。
- 1 クラス 1 ファイルを基本とし、複数クラスを 1 ファイルへまとめない。
- CMake ターゲットは次の構成を維持する。
  - `WhiteboardCore`: UI に依存しない業務ロジック。
  - `WhiteboardPlatform`: プラットフォーム依存の小さな互換ヘルパー。
  - `WhiteboardApp`: Qt Widgets の GUI。
  - `WhiteboardCoreDrawingTest`: `BoardModel` のツール選択、描画要素追加、基本図形、直線選択の Qt Test。
  - `WhiteboardCoreStyleTest`: `BoardModel` の色、塗り、線幅、線種、フォント、テキスト更新の Qt Test。
  - `WhiteboardCoreCurveTest`: `BoardModel` の折れ線、ベジェ、端点、屈曲点、閉じる/開く操作の Qt Test。
  - `WhiteboardCoreSelectionTest`: `BoardModel` の選択、複数選択、移動、リサイズ、スケール、グループ、重なり順、削除の Qt Test。
  - `WhiteboardCoreDocumentTest`: `BoardModel` の Undo/Redo、ズーム、ページ、保存読み込みの Qt Test。
  - `WhiteboardImageExporterTest`: PNG/SVG の範囲、背景、選択要素、SVG ID、拡張子、保存処理の Qt Test。
  - `WhiteboardImageExportDialogTest`: 画像保存設定ダイアログの既定値と操作部品の Qt Test。
  - `WhiteboardPersistentToolBarTest`: ツールバー拡張パネルのクリック開閉、マウス退出後の維持、
    拡張項目の操作、リサイズ時の再配置を確認する Qt Test。
  - `WhiteboardCanvasCurveTest`: `CanvasWidget` の折れ線・ベジェ操作の Qt Test。
  - `WhiteboardCanvasShapeTextTest`: `CanvasWidget` の図形ドラッグ、テキスト編集、描画確認の Qt Test。
  - `WhiteboardCanvasSelectionTest`: `CanvasWidget` の選択、頂点移動、ショートカット、点追加削除操作の Qt Test。
  - `WhiteboardMcpTest`: MCP のツール公開、図の一括反映、PNG 描画、Undo、入力拒否の Qt Test。
  - `WhiteboardMcpHttpTest`: Streamable HTTP の接続制限、セッション、protocol version、図反映の Qt Test。
- `WhiteboardCore` は `Qt6::Core` と `Qt6::Gui` だけに依存し、`Qt6::Widgets` に依存しない。
- `WhiteboardPlatform` は `Qt6::Widgets` に依存してよい。
- `WhiteboardApp` と `WhiteboardCanvas*Test` は `WhiteboardCore` と `Qt6::Widgets` に依存する。
- `WhiteboardApp`、`WhiteboardMcpTest`、`WhiteboardMcpHttpTest` は
  `/usr/local/qt/qtmcpserver` の `qtmcpserver` に依存する。
- `WhiteboardApp` と `WhiteboardMcpHttpTest` は `Qt6::HttpServer` と `Qt6::Network` に依存する。
- `WhiteboardCore*Test` は `WhiteboardCore` と `Qt6::Test` に依存し、`Qt6::Widgets` に依存しない。
- アイコン資産は `qt_add_resources()` で `/` prefix に登録し、`assets/icons/*.svg` と
  `assets/appicon/whiteboard-app-icon.png` を含める。
- macOS バンドルには `assets/appicon/WhiteboardApp.icns` を含め、
  `MACOSX_BUNDLE_ICON_FILE` に設定する。
- macOS では `CMAKE_OSX_ARCHITECTURES` の既定値を `arm64;x86_64` とし、
  Apple Silicon と Intel Mac の universal binary としてビルドする。
  この設定は macOS だけに適用し、Windows/Linux のビルド設定へ影響させない。

## クラス責務

### `BoardModel`

- UI に依存しない業務ロジックの中心とする。
- `BoardDocument` を保持し、ページ、設定、描画要素、選択状態、Undo/Redo を管理する。
- 選択状態は保存しない。`loadFromFile()`、ページ切替、Undo/Redo、対象外ツールへの切替で
  選択状態をクリアする。
- 複数選択は `m_selectedElementIndexes`、フォーカス要素は `m_selectedElementIndex` で保持する。
- 単一選択と複数選択の公開 API は既存の `BoardModel.h` の関数群を維持する。
- 編集開始・編集中・編集終了は `beginSelectionEdit()` と `endSelectionEdit()` で管理し、
  ドラッグ中の連続更新は 1 つの Undo スナップショットにまとめる。
- `pushUndoState()` は変更前に呼び、変更が成立したときだけ `clearRedoState()` を呼ぶ。
- Undo/Redo 実行時は選択状態をクリアし、文書全体のスナップショットを戻す。
- 現在ページがロック中の場合、描画追加、削除、貼り付け、移動、リサイズ、頂点編集、
  テキスト/スタイル変更、グループ化、Undo/Redo、ページ削除を成立させない。

### `BoardDocument`

- ページ配列、現在ページ index、`BoardSettings` を保持する。
- ページ数は最大 20 件に制限する。
- ページ配列が空になる場合は空ページ 1 件へ補正する。
- `currentPage()` の const / non-const アクセサを提供し、`BoardModel` は現在ページ操作にこれを使う。

### `Page`

- 描画要素配列を保持する。
- `locked` を保持する。ロック中ページは選択とコピーの対象にはできるが、ページ内容を変更できない。
- hit-test、矩形 hit-test、要素追加、複数追加、削除、並び替え用の要素配列更新を担当する。
- `hitTest()` は後ろの要素から前へ調べ、重なりの前面要素を優先する。
- `hitTestRect()` はラバーバンド矩形に bounds が含まれる要素を選択対象にする。

### `DrawingElement`

- 描画要素 1 個の型、点列、矩形、テキスト、色、塗り潰し、線幅、角丸矩形の丸み、
  線種、フォント、閉じた折れ線/ベジェ曲線、矢印、円弧角を保持する。
- `bounds()`、`moveBy()`、`resizeTo()`、`intersects()`、JSON 変換を担当する。
- 円と円弧は常に正方形 bounds に補正する。正方形化は短辺に合わせ、左上を基準にする。
- ベジェ曲線は通過点列から滑らかな曲線として扱う。2 点の場合は直線、3 点以上は
  Catmull-Rom 風に各区間を cubicTo へ変換する。
- 円弧の選択判定は円弧パスを `QPainterPathStroker` で太らせて判定する。
- 閉じた折れ線/ベジェ曲線で塗り潰し alpha が 0 より大きい場合は内部 hit-test も有効にする。

### `DrawingRenderer`

- `DrawingElement` の画面表示と PNG 出力で共有する `QPainter` 描画を担当する。
- 自由曲線、直線、折れ線、ベジェ曲線、矩形、角丸矩形、楕円、円、円弧、テキスト、
  塗り潰し、点線、回転、端点矢印を同じ描画規則で処理する。
- `CanvasWidget` と `ImageExporter` に同じ描画実装を重複させない。

### `ImageExporter`

- `WhiteboardCore` に属し、Qt Widgets に依存せず、現在ページの PNG/SVG 出力を担当する。
- 選択 index が空の場合は全要素、選択 index がある場合は選択要素だけをページ内の
  重なり順で出力する。
- `DrawingRenderer` で透明な一時画像へ描画し、alpha のある画素範囲を走査する。
  線幅、回転、矢印、アンチエイリアス画素が欠けないよう外側へ 1px の安全余白を加え、
  PNG と SVG で同じ整数座標の範囲を使う。
- PNG はキャンバス座標 1 単位を 1px として `QImage` へ描画する。
- SVG は文字列としてベクター要素を生成し、ラスター画像を埋め込まない。
  描画要素の `id` は要素種別とページ内の 1 始まり index を組み合わせる。
- 書き込みは `QSaveFile` を使い、PNG/SVG とも途中書き込みを完成ファイルとして残さない。
- 拡張子がない場合は選択形式を付加し、`.png` / `.svg` がある場合は大文字小文字を
  区別せず拡張子側の形式を優先する。

### `ImageExportDialog`

- 画像形式 PNG/SVG と背景の透明/白を選ぶモーダルダイアログとする。
- 既定値は PNG と透明背景とする。
- 「保存」と「キャンセル」は標準 `QDialogButtonBox` を使う。

### `CanvasWidget`

- キャンバス描画とマウス・キー操作の状態機械を担当する。
- `BoardModel` の業務状態を直接描画・操作し、変更後は `notifyChanged()` で再描画、
  サイズ更新、変更 callback を呼ぶ。
- 一時操作状態は `CanvasWidget` に保持する。`BoardModel` へ一時描画状態を持ち込まない。
- 維持する一時状態:
  - ペンの現在ストローク。
  - 折れ線入力中の点列。
  - ベジェ入力中の点列。
  - 折れ線・ベジェ延長時の既存点列、始点側延長か終点側延長か。
  - ドラッグ開始点、現在点、最終ドラッグ点。
  - ラバーバンド矩形。
  - リサイズ開始矩形、アンカー、ハンドル種別。
  - 回転中心、ドラッグ開始角、表示角。
  - 選択移動、リサイズ、回転、頂点移動、円弧角変更、ラバーバンドの `DragMode`。
  - その場テキストエディタと既存テキスト編集中フラグ。
- `CanvasWidget` は `QWidget` 派生のままとし、QGraphicsView など別アーキテクチャへ置き換えない。
- 画像保存開始時は `commitPendingTextEdit()` でその場テキスト編集を確定する。
  折れ線、ベジェ、図形の作成途中プレビューはモデルへ未反映のため画像へ含めない。

### `MainWindow`

- アプリ全体の UI、ツールバー、メニュー、ショートカット、保存読み込み、ウィンドウサイズ保存を担当する。
- `BoardModel` と `CanvasWidget` を所有する。
- コピー/ペースト用のアプリ内クリップボードは `QVector<DrawingElement>` として `MainWindow` が保持する。
- 中央ウィジェットは `QScrollArea` を直接 `setCentralWidget()` せず、重ね合わせ用の親 `QWidget` に
  `QGridLayout` で `QScrollArea` を配置し、右端のフローティングアクションボタンを
  同じ親 `QWidget` 上へ手動配置する。
- ツールボタンの 2 秒後ヘルプ表示と 3 秒後非表示は `MainWindow` の event filter と
  2 個の `QTimer` で実装する。
- 通常起動時は `WhiteboardMcpHttpServer` を `127.0.0.1:8765/mcp` で開始する。
  `--mcp` 指定時は互換用の標準入出力 `WhiteboardMcpController` だけを起動する。
- MCP の図反映と Undo は `QMessageBox` でユーザー承認を得てから実行し、変更後は通常の
  保存処理を呼ぶ。HTTP Server の開始失敗は警告し、GUI 本体の起動は継続する。
- メニューバーは File、Edit、Help の順とし、File に `Save Image...` を置く。
  画像保存アクションは現在ページに要素がある場合、または確定前のテキスト編集欄がある場合に
  有効とし、ページロック中でも無効化しない。
- `Save Image...` は `ImageExportDialog` の後に標準 `QFileDialog` を表示する。
  選択形式を初期フィルターと既定拡張子に使い、上書き確認は `QFileDialog` に任せる。
- 未対応拡張子は PNG/SVG を指定するまで保存せず、保存失敗時は同じ設定、保存先、選択要素で
  Retry または Cancel を選べるようにする。

### `WhiteboardMcpController`

- `QMcpServer` を所有し、標準入出力または 1 HTTP セッション分のメッセージ処理を提供する。
- builtin tool pack は `CustomOnly` とし、QObject/QEvent の汎用操作を公開しない。
- 公開ツールは状態・ページ一覧、ページ移動・追加・削除・ロック、図の一括反映、
  現在ページ PNG、PNG/SVG の取得・保存、Undo/Redo の Whiteboard 固有 12 件に限定する。
- MCP JSON と `DrawingElement` の変換、入力上限、キャンバス境界の検証を担当する。
- 図反映は全入力を変換・検証した後に `BoardModel::applyElementsToPage()` を 1 回だけ呼ぶ。
- ページ移動は callback で `MainWindow` の通常操作と同じスクロール位置保存・画面更新を行う。
- GUI 更新と保存は callback で `MainWindow` へ通知し、コントローラから直接行わない。

### `WhiteboardMcpHttpServer`

- Qt HttpServer を使い、loopback の `/mcp` で Streamable HTTP を提供する。
- initialize ごとに独立した `WhiteboardMcpController` と推測困難な session ID を作成する。
- 後続 POST では session ID と交渉済み protocol version を検証し、通知は HTTP 202 を返す。
- localhost 系以外の Origin、上限を超える本文、過剰なセッションを拒否する。
- GET の SSE ストリームは提供せず HTTP 405 を返し、DELETE でセッションを終了する。

### `PlatformColorDialog`

- 塗り潰し色選択のプラットフォーム依存処理を集約する。
- Windows で初期色 alpha 0 の場合だけ、`QColorDialog::getColor()` に渡す初期色を
  視認可能な alpha へ補正する。
- ダイアログから返った色は alpha 0 を含めて改変しない。
- UI 側に同等の Windows 分岐を重複実装しない。

## 操作状態遷移

### 選択ツール

- 左クリック時は、折れ線/ベジェ延長開始、閉じた折れ線の頂点から開く操作、
  回転ハンドル、円弧角ハンドル、頂点ハンドル、リサイズハンドル、移動、ラバーバンドの順に判定する。
- Control または Meta を押したクリックは、折れ線/ベジェ端点延長を優先する。
  延長対象がない場合は `addToSelectionAt()` で既存選択へ追加する。
- オブジェクトがない場所で左ドラッグを始めると `DragMode::RubberBand` に入り、
  ドラッグ中も `selectInRect()` で選択表示を更新し、マウスリリースで確定する。
- 選択済み要素の bounds 付近をドラッグした場合は移動とし、複数選択時も全選択要素を移動する。
- 矩形、角丸矩形、楕円、円は角ハンドルに加えて上下左右辺ハンドルを持つ。
- 直線は実際の 2 端点を頂点ハンドルとして扱い、端点ドラッグでは矩形 bounds の
  正規化で端点を入れ替えず、ドラッグした端点だけを移動する。
- 折れ線とベジェは各通過点を頂点ハンドルとして扱う。
- 円弧は始点角と終点角の 2 つの角度ハンドルを持つ。
- 選択中に修飾なし矢印キーを押すと、選択要素を 1px 移動する。
- 回転ドラッグ開始時に選択範囲中心と各要素の初期状態を `BoardModel` に保存し、ドラッグ中は初期状態からの絶対角度で再計算する。Shift 押下中は 15 度単位にする。
- 点ベース要素は点座標を回転し、矩形ベース要素は `rotationDegrees` と中心位置を更新する。矩形ベース要素のリサイズは回転角を逆変換したローカル座標で計算する。

### 折れ線

- クリックごとに点を追加する。
- ダブルクリックまたは Enter で確定する。
- Esc でキャンセルする。
- 3 点以上あり、始点または延長時の反対側端点付近をクリックした場合は閉じた折れ線として確定する。
- Ctrl+Enter または Ctrl+Shift+C は閉じた折れ線として確定する。
- Ctrl+Shift+O は選択済み閉じた折れ線を開く。
- Ctrl+Shift+クリックは閉じた折れ線のクリック頂点を先頭にして点列を回転し、開いた折れ線にする。
- Ctrl/Meta+開いた折れ線端点クリックで延長を開始する。
  始点側延長では追加点を既存点列の先頭に prepend し、終点側延長では append する。

### ベジェ曲線

- クリックごとに通過点を追加する。
- 2 点以上でダブルクリックまたは Enter により確定する。
- Esc でキャンセルする。
- 3 点以上あり、始点または延長時の反対側端点付近をクリックした場合は
  閉じたベジェ曲線として確定する。
- Ctrl+Enter または Ctrl+Shift+C は閉じたベジェ曲線として確定する。
- Ctrl+Shift+O は選択済み閉じたベジェ曲線を開く。
- 閉じたベジェ曲線は循環した点列として滑らかに描画し、端点矢印を持たない。
- Ctrl/Meta+端点クリックで既存ベジェの延長を開始する。
  始点側延長では追加点を既存点列の先頭に prepend し、終点側延長では append する。

### テキスト

- テキストはキャンバス上の `QPlainTextEdit` でその場編集する。
- 新規テキストは `Tool::Text` のクリック位置に作成する。
- 既存テキストは選択ツールのダブルクリックで編集する。
- 回転済みテキストの編集時は元の描画を一時的に隠し、回転後の視覚範囲へ水平な編集欄を表示する。確定後も回転角を維持する。
- Ctrl+Enter、Ctrl+Return、Meta+Enter、Meta+Return、フォーカスアウトで確定する。
- Esc は編集を破棄し、既存要素を変更しない。
- 空白だけの新規テキストは作成しない。

## 描画契約

- キャンバスは白背景、モデルのズーム倍率で scale して描画する。
- 線幅 0 は Qt の cosmetic pen ではなく `Qt::NoPen` として扱う。
- 線端と結合は丸くする。
- 点線/実線は `StrokeStyle` から Qt の pen style へ変換する。
- 矢印は直線、開いた折れ線、開いたベジェ曲線、円弧だけに描画する。閉じた折れ線と閉じたベジェ曲線には描画しない。
- 塗り潰しは矩形、角丸矩形、楕円、円、閉じた折れ線、閉じたベジェ曲線だけに適用する。
- 選択表示は、各選択要素の視覚範囲へ白い halo とティールの破線枠を描く。単一の矩形ベース要素は回転した輪郭に沿って描く。
- 複数選択時は個別 bounds の選択枠だけを描き、ハンドルは描かない。
- 単一選択時だけハンドルを描く。
  - 矩形、角丸矩形、楕円、円は角 4 点と辺 4 点。
  - 直線は実際の 2 端点。
  - 折れ線、ベジェは各通過点。
  - 円弧は角 4 点に加えて始点角・終点角。
- 円以外の単一選択と複数選択には回転ハンドルを描く。複数選択時のリサイズハンドルは描かない。
- `DrawingElement` の JSON は `rotationDegrees` を保存し、フィールドがない既存データは 0 度として読み込む。

## 画像保存契約

- 対象は現在ページだけとし、ズーム倍率とスクロール位置を参照しない。
- 未選択時は現在ページの全要素、選択時は選択要素だけを出力する。
- 空ページでは画像保存アクションを無効にする。
- 背景は透明を既定とし、白を選択できる。白は `#ffffff` とする。
- 選択枠、各種ハンドル、ラバーバンド、作成途中プレビューは出力しない。
- SVG は `image` 要素を生成せず、各 `DrawingElement` を対応する SVG 図形または path、text
  として出力する。要素ごとの外側 `g` に分かりやすい一意な ID を付ける。
- 画像保存は文書、選択状態、Undo/Redo 履歴を変更しない。

## ツールバーと UI 契約

- ツールバーは `MainWindow::createToolbar()` で構築する。
- `PersistentToolBar` はツールバー項目の実ウィジェットを1組だけ所有し、表示幅に応じて先頭側を
  通常行、収まらない末尾側を拡張パネルへ配置する。操作状態や signal 接続を複製しない。
- 拡張ボタンは幅に収まらない項目がある場合だけ表示し、クリックするたびにパネルを開閉する。
  マウスカーソルの退出やパネル内の操作では閉じず、幅が広がって全項目が収まった場合は閉じる。
- 拡張パネルは `MainWindow` 上へ必要な矩形だけ重ね、透明な領域や画面全体の input mask を作らない。
  Qt の内部拡張ボタン `qt_toolbar_ext_button` や内部 `QMenu` は操作しない。
- 拡張パネルの行分割は、各行について左右余白、操作部品の実幅、部品間隔を累積して判断する。
  `setFixedWidth()` などで最大幅が指定された操作部品は、`sizeHint()` より最大幅を優先して累積する。
  外側は `QVBoxLayout`、各折り返し行はスペーサーなしで左寄せした `QHBoxLayout` とする。項目配置後の
  最終パネル幅は外側レイアウトの `sizeHint()` にフレーム幅を加えた値とし、必要幅を超える余剰幅を
  Qt のレイアウトエンジンへ渡さない。複数行で列幅を共有する `QGridLayout` は使用しない。
- macOS では管理対象の操作部品へ共通登録処理で `Qt::WA_LayoutUsesWidgetRect` を設定し、独自の
  幅計算で使うウィジェット矩形と Qt のレイアウト項目矩形を一致させる。
- ページ操作グループはページ番号ラベル、Previous、Next、Add、Delete、Lock の順に登録し、Lock を
  ツールバーの末尾項目にする。ページ番号ラベルは横方向を `Fixed`、文字を右寄せとし、表示更新後に拡張配置を再計算して、
  通常表示とロック表示の `sizeHint()` にパネル右端を追従させる。背景の右余白には外側 `QToolBar`
  左端から通常行の先頭操作部品までの実測幅を左右の `contentsMargins` に使う。左右余白を含む
  外側 `QVBoxLayout::sizeHint()` を最終幅に使うことで、先頭項目の左端と末尾項目の右端からパネル端までの
  幾何学的余白を同じにする。
- ツールバーの `QToolButton` には `toolbarRole` dynamic property を設定し、
  基本ツール、図形ツール、テキスト、編集、スタイル、ズーム、ページ操作を
  役割別の落ち着いた背景色で色分けする。基本ツールはシアン系、図形はグリーン系、
  テキストはイエロー系、編集操作はブルー系、スタイル設定はオレンジ系、ズームは
  シアン系、ページ操作はローズ系、削除など破壊的操作はより強い赤系にする。
- ツールバー上の無効化された `QToolButton` は、`toolbarRole` ごとのカテゴリ色を保ち、
  背景色でカテゴリを判別できるようにする。有効時や hover 時とは、控えめな枠線と
  アイコンの薄さで区別し、暗い背景へ沈ませない。
- フローティングアクションは `MainWindow::createFloatingActions()` と
  `FloatingActionButton` で構築する。初期状態では右端の丸いメインボタンだけを表示し、
  ユーザーはメインボタンを上下ドラッグできる。
  クリックすると Select、Reset Style、Undo、Redo、Copy、Paste の各ボタンをこの順序で展開する。
  メインボタンが画面上半分にある場合は子ボタンを下方向へ、下半分にある場合は上方向へ展開する。
  クリックとドラッグの判定は `FloatingActionButton` が `activated()` と `draggedTo(int)` で通知する。
  `MainWindow` は配置、表示、操作ヘルパー呼び出しだけを担当する。
  展開・折りたたみは `QPropertyAnimation` で子ボタン用コンテナの `geometry` を 20px
  スライドさせ、`QGraphicsOpacityEffect` の `opacity` も同時に変化させる。
  高さ 0 から広げるアニメーションや input mask は使わない。
  透明領域がキャンバス操作を遮らないように、前面パネルはメインボタンと展開中の子ボタン群を
  包む最小矩形へ都度更新する。
  吹き出しラベル表示のためにフローティングアクションの前面パネル幅を広げない。
  パネルの見えない領域がキャンバス右側のオブジェクト選択を奪わないことを優先する。
  メインボタンの Y 位置は `BoardSettings::floatingActionDockY` として保存する。
  Undo と Redo は連続操作しやすいように、クリック後も展開状態を維持する。
  これらはツールバーと同じ操作ヘルパーを呼び、別実装に分岐させない。
- フローティングアクションの子ボタンは `FloatingActionChildButton` で構築する。
- 「選択」が常に親 FAB の隣になるよう、FAB が親ウィジェットの下半分にあり上方向へ展開する場合はレイアウト順とタブ移動順を反転する。展開中のドラッグでも位置更新時に判定し、画面中央をまたいだ場合は展開方向と順番を更新する。
  hover 時はボタンサイズやアイコンサイズを変えず、ボタン列がガタつかないようにする。
  操作名は `FloatingActionBubbleLabel` で子ボタン左横に表示する。
  ラベルは黒系背景、白文字、右端に子ボタンへ向いた尖りを持つ。
  ラベルの親はキャンバス側の親ウィジェットにし、ラベル自身は `WA_TransparentForMouseEvents` を設定して
  マウス操作を受け取らない。
  ラベル幅は固定上限で切らず、ボタン左側の空き幅と `sizeHint()` に基づいて決める。
  FAB 子ボタンの操作名表示は標準 `QToolTip` に依存せず、吹き出しラベルに一本化する。
- UI 文字列は英語をソース上の基準文字列とし、日本語は `translations/whiteboard_ja.ts` に定義する。
  CMake は `LinguistTools` と `qt_add_translations()` を使い、生成された `whiteboard_ja.qm` を
  `:/i18n/whiteboard_ja.qm` としてリソースに埋め込む。
  起動時に `QLocale::system().language() == QLocale::Japanese` の場合だけ `QTranslator` で日本語翻訳を読み込む。
  日本語以外の環境では英語基準文字列をそのまま表示する。
- 背景はチャコールグレー系の `#3f464d`、下境界線 `#242a30`、細い separator `#68737d`、
  控えめな drop shadow を持つ。
- ツールボタンは 24x24 SVG アイコンを使い、チェック状態を QActionGroup で排他制御する。
- グループ順は次を維持する。
  - 基本ツール: Select, Pen, Eraser
  - 図形ツール: Line, Rectangle, Rounded Rectangle, Circle, Ellipse, Polyline, Bezier, Arc
  - テキスト関連: Text, Font
  - 編集操作: Undo, Redo, Copy, Paste, Send Backward, Bring Forward, 折れ線 close/open ショートカット
  - スタイル設定: Color, Fill, Reset Style, 線幅, 角丸矩形の丸み, 線種, 開始矢印, 終了矢印
  - 表示操作: Zoom Out, ズーム spin box, Zoom In
  - ページ操作: ページ表示 label, Previous, Next, Add, Delete, Lock
- ズーム spin box は `keyboardTracking(false)` とし、`editingFinished` でのみ倍率へ反映する。
- ツールバー上の `QSpinBox` と `QComboBox` は薄灰背景、角丸、控えめな枠線の入力ピルとして
  同じ見た目に統一する。フォーカス時はティール枠にする。
- ツールバー入力部品は内容に合わせた固定幅にし、右側の余白が目立たないようにする。
- ツールバー上の `QComboBox` は高さを 44px に固定し、左右ボタン付き `QSpinBox` と
  見た目の高さのバランスをそろえる。
- ツールバー上の `QComboBox` は本体幅を固定しつつ、ポップアップ幅は項目文字列が切れない
  幅へ広げる。
- ツールバー上の `QComboBox` は `CenteredComboBox` を使い、閉じた状態の表示文字だけを
  `paintEvent()` で中央揃えにする。editable 化やクリックイベントの横取りで中央揃えを
  実装せず、ポップアップの開閉と選択は標準 `QComboBox` の挙動を維持する。
- ツールバー上の `QSpinBox` は `createSteppedSpinBox()` で左右に減少/増加ボタンを付けた
  分割ボタン型にし、スピンボックス本体の標準上下ボタンは表示しない。
  左右ボタンは `stepDown()` / `stepUp()` だけを呼び、スピンボックスの有効/無効状態に同期する。
- 線幅 spin box は 0 から 32 を許可する。
- 角丸矩形の丸み spin box は 0 から 200 を許可する。角丸矩形選択中は
  `updateSelectedCornerRadius()` で選択要素へ反映し、それ以外は
  `setCornerRadius()` で以後作成する角丸矩形の既定丸みを変更する。
- 線種 combo box は「実線」「点線」を表示する。
- 矢印 combo box は、開始端点側は「始: なし」「始: 三角」「始: 開き」「始: ひし形」、
  終了端点側は「終: なし」「終: 三角」「終: 開き」「終: ひし形」を表示する。
  item data は `ArrowHead` の値を保持し、表示接頭辞をロジックへ持ち込まない。
- Color は、選択中要素がある場合は対象要素の色を初期値にする。テキスト選択中は
  `updateSelectedTextColor()` で文字色へ反映し、テキスト以外の描画要素選択中は
  `updateSelectedColor()` で描画色または輪郭線色へ反映する。未選択の場合だけ
  `setSelectedColor()` で以後作成する描画要素の既定描画色を変更する。
- ページ削除は確認ダイアログを出す。
- 保存ファイルはアプリデータ領域のローカル JSON とする。

## ショートカット契約

- ツール選択:
  - Select `Ctrl+1`
  - Pen `Ctrl+2`
  - Eraser `Ctrl+3`
  - Line `Ctrl+4`
  - Rectangle `Ctrl+5`
  - Circle `Ctrl+6`
  - Text `Ctrl+7`
  - Polyline `Ctrl+8`
  - Bezier `Ctrl+9`
  - Arc `Ctrl+0`
- Delete / Backspace は選択要素削除。
- Copy / Paste は標準キーを使い、macOS の Command-C / Command-V でも動くよう
  `QKeySequence::Copy` / `QKeySequence::Paste` を使う。
- Undo / Redo は標準キーを使い、Redo は `Ctrl+Shift+Z` も登録する。
- z-order の 1 段移動は `Ctrl+[` と `Ctrl+]`。
- z-order の最背面/最前面移動は `Ctrl+Shift+[` と `Ctrl+Shift+]`。
- ページ移動は `Alt+Left` / `Alt+Right`。
- ページ追加は `Ctrl+Shift+N`。
- ページ削除は `Ctrl+Shift+Backspace`。
- 折れ線/ベジェ曲線を閉じる操作は `Ctrl+Shift+C`。
- 折れ線/ベジェ曲線を開く操作は `Ctrl+Shift+O` と `Meta+Shift+O`。
- ツール選択、z-order、ページ追加、ページ削除のショートカットは
  `MainWindow::eventFilter()` で Control または Meta を明示的に扱い、macOS でも
  物理 Control キーで動作させる。
- 同じ理由で、ツール選択、z-order、ページ追加、ページ削除の `QAction` には
  `Ctrl+...` と `Meta+...` の両方を登録する。

## 保存 JSON 契約

- 保存は `BoardDocument::toJson()` を `QJsonDocument::Indented` で書き込む。
- ルートキーは次を維持する。
  - `pages`
  - `currentPageIndex`
  - `settings`
- 各ページのキーは次を維持する。
  - `zoomPercent`
  - `scrollX`
  - `scrollY`
  - `locked`
  - `elements`
- `settings` のキーは次を維持する。
  - `zoomPercent`
  - `selectedTool`
  - `selectedColor`
  - `fillColor`
  - `strokeWidth`
  - `cornerRadius`
  - `strokeStyle`
  - `fontFamily`
  - `fontPointSize`
  - `windowWidth`
  - `windowHeight`
  - `startArrowHead`
  - `endArrowHead`
- 描画要素のキーは次を維持する。
  - `type`
  - `points`
  - `rect`
  - `text`
  - `color`
  - `fillColor`
  - `strokeWidth`
  - `cornerRadius`
  - `strokeStyle`
  - `fontFamily`
  - `fontPointSize`
  - `closed`
  - `startArrowHead`
  - `endArrowHead`
  - `arcStartAngle`
  - `arcSpanAngle`
- 色は `QColor::HexArgb` 形式で保存する。
- 選択状態、Undo/Redo スタック、一時入力状態は保存しない。

## 入力検証と補正

- ズーム倍率は `BoardModel::MinZoomPercent` から `BoardModel::MaxZoomPercent` にクランプする。
- 線幅は 0 以上に補正する。
- 角丸矩形の丸みは 0 以上 200 以下に補正する。
- ウィンドウサイズは有効な場合だけ保存し、幅 800、高さ 600 を下限にする。
- テキストは空白だけなら追加・更新しない。
- 円と円弧は正方形に補正する。
- 円弧 span は `-359..359` に制限し、0 の場合は 1 に補正する。
- ページは最大 20 件、最小 1 件を維持する。
- MCP の図反映は最大 500 要素、点ベース要素は 1 要素最大 1000 点、テキストは最大
  10000 文字とする。数値は有限値で、要素の視覚範囲全体がキャンバス内にあることを要求する。
- MCP 入力は既存の対話操作と異なり、不正値を暗黙補正せず要求全体を拒否する。

## 再生成時の検証

- 再生成後は少なくとも次を実行する。
  - CMake configure。
  - ビルド。
  - `ctest --output-on-failure`。
- MCP テストでは公開ツールが 12 件であること、ページ一覧・移動・管理、画像取得・保存、
  Undo/Redo、一括反映が 1 Undo であること、PNG が 1920x1080 であること、
  不正入力とロック中ページが更新されないことを確認する。
- MCP HTTP テストでは loopback 待受、Origin 制限、initialize、session ID、protocol version、
  通知、ツール呼び出し、セッション終了を確認する。
- 画像保存テストでは選択要素だけの出力、PNG/SVG の同一範囲、透明/白背景、回転と線幅の
  欠け防止、SVG のベクター要素と ID、拡張子規則、設定ダイアログの既定値を確認する。
- ツールバー拡張テストではクリックによる開閉、マウス退出と拡張項目操作後の表示維持、
  外側 `QToolBar` での幅使用、通常行と拡張行の操作部品の左端揃え、拡張パネル背景と外側
  `QToolBar` の左端揃え、配置後の先頭・末尾ウィジェット座標を基準とする左右余白の一致、
  固定幅が `sizeHint()` より小さい操作部品の幅、
  全項目が収まる幅へ広げた場合の
  自動折りたたみを確認する。
- 現在実装との再現度確認では、テスト通過だけでなく `include/`、`src/`、
  `CMakeLists.txt` の差分も確認する。
- 差分が出る場合は、機能仕様差分、実装設計契約差分、単なる実装表現差分を分けて報告する。
