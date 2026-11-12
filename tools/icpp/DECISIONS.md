# DECISIONS

## 採用済み

- icpp の MCP 対応は、通常の対話 REPL を既定のまま維持し、`--mcp-http` を指定した場合だけ Streamable HTTP Server として起動する。
- MCP HTTP endpoint は `127.0.0.1` の `/mcp` に限定し、すべての要求で `ICPP_MCP_TOKEN` による Bearer 認証を必須にする。
- 任意コード評価と interpreter reset は `--mcp-allow-execution` を指定した場合だけ許可する。HTTP Server を起動しただけではコード実行を許可しない。
- 初版は同時 1 MCP session、JSON response のみに限定する。session ごとに独立した cling / qtcling process を持つ。
- MCP 専用 interpreter process は既存 REPL の `InterpreterProcess` と分離し、stdout / stderr を捕捉する。`ICPP_MCP_TOKEN` は子 process の環境へ継承しない。
- request と各出力は 1 MiB、評価時間は 100 ms から 30 秒に制限する。timeout または crash 後は process を終了し、次の評価で新しく起動する。
- MCP protocol の処理は icpp 本体に内蔵する。`initialize`、`notifications/initialized`、`tools/list`、`tools/call` の現在必要な範囲だけを実装し、外部の QtMcpServer source tree や shared library へ依存しない。

## 不採用

- 通常の icpp 起動時に MCP HTTP Server を自動起動する構成は採用しない。
- 初版で複数 session の評価を同期 HTTP handler 上に載せる構成は採用しない。
- 初版では server 起点の SSE stream を提供せず、GET `/mcp` は HTTP 405 を返す。

## まだ決めていないこと

- MCP code evaluation を OS level で隔離する方法。
- OAuth 2.1 準拠の認可や remote host 公開を将来扱うか。

## 判断基準

- 既存の対話 REPL の挙動を変えないことを優先する。
- 任意 code execution は認証とは別の危険性を持つため、明示許可と resource 制限を必須にする。
- 複数 session や streaming より、process 終了と timeout 後の回復を優先する。

## 継続する仕様・運用制約

- ルートの `VERSION` をicppリリース番号の唯一の定義元とし、CMake project version、`icpp --version`、インストールする `share/icpp/VERSION` へ反映する。
- source配布ZIPは`release/icpp-<version>-source.zip`に置き、ZIP内のトップディレクトリを`icpp-<version>/`とする。stagingはリポジトリ外の一時ディレクトリで行う。
- CMake install では `icpp` を `${CMAKE_INSTALL_PREFIX}/bin` に配置する。MCP Server 機能は icpp 本体に含めるため、QtMcpServer shared library は配置しない。
- MCP HTTP Server は local development 専用とし、loopback 以外では listen しない。
- MCP経由で評価するC++は、icpp processと同じ利用者権限を持つ。Bearer認証はsandboxを提供しない。
- Streamable HTTPで受け付けるMCP protocol versionは `2025-03-26` と `2025-06-18`。
