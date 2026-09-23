# Qt Creator MCP 検証

Qt Creator MCP Server を使って、Qt Creator 上のアクティブプロジェクトに対する自動確認を行うための手順を置く。

この検証は通常の shell script ではなく、Codex が Qt Creator MCP tool を呼び出して実行する。CI 用ではなく、開発者ローカルで Qt Creator と Codex を接続した状態の半自動検証である。

## 前提

- Qt Creator が起動している
- `Tools -> MCP Server -> About MCP Server` で MCP Server が running になっている
- Codex 側で `qtcreator` MCP server が enabled になっている
- Codex を MCP 登録後に再起動している
- 検証用 CMake プロジェクトを Qt Creator で開き、Kit / build config を設定している

接続確認:

```text
list_projects
get_current_project
list_open_files
```

これらが空ではなく JSON を返せば、Qt Creator MCP は利用できる。

## 検証ケース

| ファイル | 内容 |
|---|---|
| `build-error-roundtrip.md` | ビルドエラーを作り、MCP で取得し、Codex 側で修正して再ビルドする |

## 判定の注意

`list_issues` は古い issue を保持する場合がある。修正後の判定では、次を組み合わせて確認する。

- Qt Creator MCP の `file_plain_text` が修正後の内容を返す
- 実ファイルに古いエラー文字列が残っていない
- `build` が完了する
- 必要に応じて Qt Creator 側で `close_file` / `open_file` して再読み込みする

QML runtime error や Application Output は、現時点の MCP tool では取得対象外とする。

## 検証用プロジェクト

`project/` に Qt Creator MCP 検証専用の最小 Qt Widgets プロジェクトを置く。

```text
verification/qtcreator-mcp/project/CMakeLists.txt
```

Qt Creator に CMake プロジェクトとして開かせるには、macOS では次のようにできる。

```sh
open -a "Qt Creator" verification/qtcreator-mcp/project/CMakeLists.txt
```

初回は Qt Creator 側で Configure Project / Kit 選択が必要になる。MCP tool には現時点で Kit を新規設定する操作がないため、ここだけは手動で行う。

