# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 現在の状態

- `icpp` 1.2.0 の配布用 ZIP `release/icpp-1.2.0-source.zip` を作成・検証済み（2026-09-02）。
  - ZIP 内トップは `icpp-1.2.0/`、収録ファイルは 337 個。
  - SHA-256 は `7267e4ddfc66d79f7a6b4e07e8ebfc1a21b181e16fa1a2d0ba43fbbbfd11a2f3`。
  - ZIP に `build/`、`release/`、過去の ZIP、`__MACOSX` が含まれないことを確認済み。
  - 展開したソースからの構成・ビルドと `icpp 1.2.0` の表示を確認済み。
  - `/Users/sugita/Sites/tools/icpp` へ ZIP、`USER_GUIDE.md`、`USER_GUIDE.pdf` を配置済み。配布ページの版番号、ZIP リンク、埋め込み利用ガイドも 1.2.0 へ更新済み。
- Streamable HTTP MCP Server 対応を実装済み。
  - MCP protocol 処理は icpp 本体に内蔵済み（2026-09-02）。
  - `/usr/local/qt/qtmcpserver` の source tree、`qtmcpserver` shared library、`QMcpServer` API には依存しない。
  - 内蔵実装は `initialize`、`notifications/initialized`、`tools/list`、`tools/call` と、既存 tool の `icpp/session/status`、`icpp/code/evaluate`、`icpp/session/reset` を扱う。
  - `--mcp-http` で `http://127.0.0.1:<port>/mcp` を明示起動する。既定 port は 8766。
  - Bearer token は `ICPP_MCP_TOKEN` を使用し、32 bytes 未満では起動しない。
  - `--mcp-allow-execution` がない場合は `icpp/session/status` だけを実行できる。
  - 許可時は `icpp/code/evaluate` と `icpp/session/reset` を使用できる。
  - 同時 1 session、30 分で失効、request / stdout / stderr は各 1 MiB、評価 timeout は最大 30 秒。
  - timeout や interpreter crash 後は子 process を終了し、次回評価時に再起動する。
  - `ICPP_MCP_TOKEN` は cling / qtcling の環境へ継承しない。
  - `cmake --build build --parallel 4` は成功済み（2026-09-02）。
  - `ctest --test-dir build --output-on-failure -R icpp_mcp_http` は成功済み（2026-09-02）。
    - 管理環境内では localhost の待受制限で起動できないため、許可された環境で再実行した。
  - テストは3件すべて成功済み（2026-08-01）。
    - `icpp_integration`: 128.31秒
    - `icpp_mcp_http`: 2.00秒。管理環境内では localhost の待受制限で起動できないため、許可された環境で再実行した。
    - `icpp_source_layout`: 0.01秒
  - 実機で `./build/icpp --mcp-http` の起動と endpoint 表示を確認済み（2026-07-31）。この起動方法では意図どおり status tool だけが使用可能。
- MCP 対応前の icpp の直近コミットは `d65e89c81 Guard incomplete source evaluation`（2026-07-13）。
  - `.load` / `.l`、`.e` 後の評価、`.r` / `.run`、`.gen` 後の再評価では、`{}` が対応していないソースを interpreter へ送信せずに停止する。
  - 登録ファイルと編集バッファを評価する共通経路 `runVisibleSourceCodeWithoutAutogen()` にも適用している。
  - 壊れた C++ によって cling が未完了入力状態になり、その後の `qApp` などが通常のトップレベル式として扱われなくなる問題を防ぐための処置。
  - 手動確認では、壊れたソースを止めた後も `qApp` を通常どおり評価できた。
  - `cmake --build build` と `git diff --check` は成功済み。
- 配布物 `icpp-1.0.1-20260601.zip` は作成済み。ZIP 内トップは `icpp/`。
- CMake install rule とインストール手順を追加した（2026-08-15、2026-09-02 更新）。
  - `sudo cmake --install build --prefix /usr/local` で `icpp` を `/usr/local/bin` に配置する。
  - MCP Server 機能は icpp 本体に含まれるため、QtMcpServer shared library の配置は不要。
  - 新しい build directory で `libedit-dev` を有効にした build、staged install、インストール済み実行ファイルの `--version` を確認済み。
  - 既存の `build/` cache が GNU Readline 用 include path を保持していた場合は、`rm -rf build` 後に再構成する。
- 技術記事 `icpp-qtcling-technical-article.html` はドラフト作成済みだが、図とスクリーンショットの最終素材への差し替えが残っている。
- `build/` と `button.cpp`、`click_message_button.cpp`、`examples/11_widget_gallery.cpp`、`s.cpp`、`saved_quit.cpp` は未追跡の生成物・試行用ファイルとして残っている。コミット対象に含めない。

## 次にやること

1. Codex を実際の MCP client として接続し、`icpp/session/status` を確認する。
   - Codex 側は `codex mcp add icpp --url http://127.0.0.1:8766/mcp --bearer-token-env-var ICPP_MCP_TOKEN` で登録する。
   - icpp と Codex の起動環境へ同じ `ICPP_MCP_TOKEN` を設定し、登録後に Codex を再起動する。
   - code evaluation を確認する場合だけ、icpp を `--mcp-allow-execution` 付きで起動する。
2. QML / Qt Quick 対応の実装単位を決める。
   - このファイルに残っていた案は、`icpp` と同一バイナリにして実行名 `iqml` で mode を切り替える構成。
     - `icpp`: C++ / QWidget REPL
     - `iqml`: QML / Qt Quick preview REPL
     - install 時に `iqml -> icpp` の symbolic link を作る。
   - REPL 入出力、line editor、history、command registry、help、editor 起動、buffer / file 管理、clipboard、外部コマンド、settings は共通化する案。
   - QML preview、reload、QML logs / errors、import path、qmllint / qmlformat、Qt Quick object inspection は `iqml` mode 固有にする案。
   - QtQuick 依存が `icpp` 側に重すぎる場合は、共通 library + `icpp` / `iqml` 別 binary に分ける。
   - `../iqml/DESIGN.md` / `../iqml/NEXT.md` は Qt ネイティブ host app の構想を記載しているが、同一バイナリに統合するかは明記していない。
   - 実装前に構成を一本化し、v0.1 の範囲について許可を得る。
3. `QTCREATOR_GUIDE.md` / PDF に、廃止済みの `.workflow` が残っている。現行の `.where` / `.qt` / `.generated` / `.doctor` を使う説明へ直す。
4. incomplete source guard を次に変更する場合は、`.load` と登録ファイル再評価の自動回帰テストを追加する。

## 未完了

- MCP Server は自動テストと手動起動を確認済みだが、実際の MCP client からの接続確認は未実施。
- `iqml` の統合方針と v0.1 scope は未決定。決定前にコードや `CMakeLists.txt` を変更しない。
- `QTCREATOR_GUIDE.md` と PDF の `.workflow` 記述が現行仕様と一致していない。
- incomplete source guard は手動確認済みだが、専用の自動回帰テストはない。
- その他の保留事項は `backlog.md` を参照する。

## 参照するファイル

- `src/McpHttpServer.cpp`
- `src/McpController.cpp`
- `src/McpInterpreterSession.cpp`
- `tests/tst_mcp_http.cpp`
- `docs/mcp_icpp_capabilities.md`
- `DECISIONS.md`
- `src/ReplSessionInterpreter.cpp`
- `src/ReplSessionEdit.cpp`
- `include/icpp/ReplSessionImpl.h`
- `backlog.md`
- `../iqml/DESIGN.md`
- `../iqml/NEXT.md`

## 注意

- リポジトリ root は `/Users/sugita/src`。repo 全体には無関係な未追跡ファイルが多いため、コミット時に `git add .` は使わず、icpp 関連ファイルだけを stage する。
- `CMakeLists.txt` を変更する前に `~/AGENTS.cmake.md` を読む。
- 指示にない機能追加や仕様変更は、内容・必要性・既存挙動への影響を示して許可を得てから実装する。
