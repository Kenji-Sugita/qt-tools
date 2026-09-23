# backlog

`NEXT.md` の近傍から外れた残作業、問題点、保留事項を記録する。
完了履歴はここへ保存せず、Git の履歴を参照する。

## テスト

- incomplete source guard の自動回帰テストを追加する。
  - `.load` / `.l` で `{}` が対応しないソースを拒否する経路。
  - `.r` / `.run` で登録ファイルまたは編集バッファを拒否する経路。
  - 拒否後も interpreter が未完了入力状態にならず、通常のトップレベル式を評価できること。

## MCP Server

- MCP code evaluation を未信頼 code にも使用する必要が生じた場合は、別利用者、container、VM など OS level の隔離を検討する。
- 同時に複数の MCP session を評価する必要が生じた場合は、session ごとの worker thread または非同期 process 制御を設計する。
- server 起点通知が必要になった場合は、GET `/mcp` の SSE stream、再接続、event ID、再送を実装する。
- remote host へ公開する必要が生じた場合は、TLS、OAuth 2.1 protected resource metadata、scope、token audience validation を先に設計する。
- MCP protocol `2025-11-25` 以降へ対応する必要が出た時点で、icpp 内蔵 MCP 実装の互換性と HTTP header の扱いを再検証する。

## ドキュメントと記事

- `QTCREATOR_GUIDE.md` / PDF から廃止済みの `.workflow` を除く。
  - 状態確認は `.where` / `.qt` / `.generated` / `.doctor` に分ける。
  - 手順例は `.examples` と `USER_GUIDE.md` に寄せる。
- `icpp-qtcling-technical-article.html` の図とスクリーンショットを最終素材へ差し替える。
  - HTML コメントに必要な画像のファイル名と準備内容が残っている。
- 複数ファイル機能やクリップボード機能を変更する場合は、`USER_GUIDE.md` と個別ヘルプが実装と一致しているか併せて確認する。

## 対応環境

- Windows ネイティブ VC++ 環境の line editing は保留。
  - 現行方針は GNU Readline または libedit を優先し、利用できない環境では標準入力へフォールバックする。
  - 簡易実装や linenoise 系を採用する予定はない。既存の操作性と同等の案が必要になった時点で再検討する。

## 外部エディター連携

- Emacs クライアント化は保留。
  - `inferior-icpp-mode` / `comint` から icpp を起動し、選択範囲、現在行、バッファ、現在ファイルを送る案。
  - `.add` / `.r` / `.gen` / `.runorder` などを Emacs コマンド化する案。
  - 必要性が明確になるまで icpp 本体へ専用 protocol を追加しない。将来必要なら、完了マーカーや機械処理しやすい出力形式を検討する。
- Qt Creator プラグイン化は保留。
  - 現在は Qt Creator を本体開発、icpp を横に置く確認用 REPL として使う。
  - Qt Creator API 差分、配布、保守、Output Pane / Kit 連携を含むため、必要になれば別プロジェクト級の作業として再検討する。

## 設定

- `.toml` 対応は低優先度で保留。
  - 現在は個人設定を QSettings、明示的な一時指定をコマンドラインと環境変数で扱う。
  - 将来追加する場合も、`engine`、include path、Qt tool path、PropertyEditor path など作業ディレクトリ単位の最小設定に限定する。
  - 登録ファイル順、生成物、workflow、project model は `.toml` に入れない。

## 継続する設計判断

以下は未実装項目ではないが、将来の変更で意図せず覆さないために残す。

### icpp の役割

- icpp は CLI REPL として、小さな C++ / Qt 実験、lesson、Qt 生成物の確認に集中する。
- 本格的な project model、build、debug、test は Qt Creator に任せる。
- `qtcling-project` 構想は中止済み。別 project helper を前提にしない。
- `qtcling` の GUI REPL 構想も中止済み。Qt Creator / Designer / Linguist と役割が重なる GUI 開発環境へ広げない。
- Qt Creator MCP Server は icpp 本体へ組み込まず、Qt Creator 側の build / debug / test を AI が扱う周辺ツールと位置づける。

### 編集と再評価

- 標準ワークフローは、引数なし `.e` の一時バッファより、実ファイル + `.add` / `.e <file>` / `.r` / `.x` を優先する。
- `.add` は `.cpp` / `.cc` / `.cxx` / `.c++` / `.c` の実装ファイルだけを登録する。header は実装ファイルから include し、直接登録による class redefinition を避ける。
- `.r` と `.e <file|number>` 後の再評価は interpreter を restart し、登録ファイル、編集バッファの順に評価する。
  - 対話入力だけで作った変数は restart 後に残らない。
  - 残したいコードは `.a <code>` で編集バッファへ入れるか、登録ファイルへ書く。
- 通常入力を編集バッファへ追加する `.b` の既定値は `off`。必要な行だけ残す場合は `.a` を使う。

### Qt 生成物とツール

- `.gen` は当面 `run_all` のラッパーとして維持する。
- `.workflow` は発散しやすいため廃止済み。再導入せず、状態確認を `.where` / `.qt` / `.generated` / `.doctor` に分ける。
- 翻訳は `.linguist` と明示的な `.! lupdate ...` / `.! lrelease ...` で扱う。icpp 専用 alias は作らない。
- icpp 本体、qtcling、`run_all` / `moc` / `uic` / `rcc`、QtUiTools、PropertyEditor は同じ Qt 6.x.y で揃える運用を基本とする。
- icpp 実装内で説明用の `Q_OBJECT` 文字列を書く場合は、CMake AUTOMOC の誤検出を避けるため文字列連結で扱う。

### GUI 補助機能

- `.inspect` の PropertyEditor は icpp 本体へリンクせず、実行時に qtcling へ読み込ませる。
- `.widgets` / `.closeall` は icpp 内部の inspector window と非表示の補助 window を利用者の widget として扱わない。
- `.uiinfo` は `.ui` を XML として解析するだけで、生成やロードを行わない。
- `.preview` は QUiLoader で `.ui` を直接表示し、`uic` / `.gen` / `run_all` を実行しない。
