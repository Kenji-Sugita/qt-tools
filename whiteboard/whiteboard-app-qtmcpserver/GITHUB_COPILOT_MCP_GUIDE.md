# GitHub Copilot から WhiteboardApp MCP を利用する手順

## 1. この説明書の対象

この説明書は、外部ライブラリ `/usr/local/qt/qtmcpserver` を使っていた版の
WhiteboardApp を、macOS 上の GitHub Copilot から操作する担当者向けです。

- ソースディレクトリ:
  `/Users/sugita/src/qt/tools/whiteboard/whiteboard-app-qtmcpserver`
- アプリケーション:
  `/Users/sugita/src/qt/tools/whiteboard/whiteboard-app-qtmcpserver/build/WhiteboardApp.app`
- MCP endpoint: `http://127.0.0.1:8765/mcp`
- MCP transport: Streamable HTTP
- 公開 tool 数: 12

主手順は、Mac 上で動作する Visual Studio Code の GitHub Copilot Chat を対象とします。
GitHub Copilot CLI から接続する方法も後半に記載します。

## 2. 前提条件

次を確認してください。

1. WhiteboardApp がビルド済みである。
2. `/usr/local/qt/qtmcpserver` が存在する。
3. Visual Studio Code に GitHub Copilot と GitHub Copilot Chat が導入されている。
4. GitHub Copilot へサインインしている。
5. Copilot Business または Enterprise を利用している場合、組織設定で MCP の使用が許可されている。
6. Visual Studio Code を SSH、Dev Container、Codespacesではなく、WhiteboardAppと同じMac上で実行している。

このWhiteboardAppのMCP Serverはloopback接続だけを許可します。別のPC、コンテナ、クラウド上の
CopilotからMacの `127.0.0.1:8765` へ接続することはできません。

## 3. WhiteboardAppを起動する

Finderから `WhiteboardApp.app` を開くか、ターミナルで次を実行します。

```bash
open /Users/sugita/src/qt/tools/whiteboard/whiteboard-app-qtmcpserver/build/WhiteboardApp.app
```

通常起動すると、GUIとともにMCP Serverが `http://127.0.0.1:8765/mcp` で待ち受けます。

ポートを使用できない場合はWhiteboardAppに警告が表示されます。その場合は、別の
WhiteboardAppが動いていないか確認してください。

```bash
lsof -nP -iTCP:8765 -sTCP:LISTEN
```

<div class="page-break"></div>

## 4. Visual Studio CodeにMCP Serverを登録する

### 4.1 ワークスペース単位で登録する場合

Visual Studio Codeで利用対象のプロジェクトを開き、そのプロジェクトに
`.vscode/mcp.json` を作成します。

```json
{
  "servers": {
    "whiteboard": {
      "type": "http",
      "url": "http://127.0.0.1:8765/mcp"
    }
  }
}
```

このファイルを共有リポジトリへコミットするかどうかは、プロジェクトの運用方針に従ってください。

### 4.2 利用者共通設定へ登録する場合

複数のワークスペースで利用する場合は、Visual Studio Codeのコマンドパレットを開き、
`MCP: Open User Configuration` を実行します。開いた `mcp.json` の `servers` に、上記と同じ
`whiteboard` 設定を追加してください。

### 4.3 接続を開始する

1. WhiteboardAppが起動していることを確認する。
2. `mcp.json` を保存する。
3. `mcp.json` 上部に表示される `Start` を押す。
4. GitHub Copilot Chatを開く。
5. Chatのモードを `Agent` にする。
6. Chatのツールアイコンを開く。
7. `whiteboard` と、その配下のtoolが表示されることを確認する。

## 5. 接続確認

Copilot Chatへ次のように入力してください。

```text
Whiteboard MCPを使って、現在のホワイトボードの状態を取得してください。
```

成功した場合は、現在ページ、ページ数、ロック状態、選択状態、Undo/Redo状態などが返ります。

続いて、読み取りとページ移動を確認します。

```text
Whiteboard MCPを使ってページ一覧を取得してください。
```

```text
Whiteboard MCPを使って次のページを表示してください。
```

<div class="page-break"></div>

## 6. 利用できるtool

| MCP tool | 内容 | WhiteboardApp側の確認 |
| --- | --- | --- |
| `whiteboard/state` | 現在の状態を取得 | 不要 |
| `whiteboard/pages/list` | ページ一覧を取得 | 不要 |
| `whiteboard/page/navigate` | 前後のページへ移動 | 不要 |
| `whiteboard/page/add` | 空ページを追加 | 必要 |
| `whiteboard/page/delete` | 現在ページを削除 | 必要 |
| `whiteboard/page/lock` | 現在ページのロックを変更 | 必要 |
| `whiteboard/diagram/apply` | 図を一括反映 | 必要 |
| `whiteboard/page/render` | 現在ページを1920×1080 PNGで取得 | 不要 |
| `whiteboard/image/export` | PNG/SVGデータを取得 | 不要 |
| `whiteboard/image/save` | PNG/SVGファイルを保存 | 必要 |
| `whiteboard/history/undo` | 直前の変更を元に戻す | 必要 |
| `whiteboard/history/redo` | Undoした変更をやり直す | 必要 |

変更を伴うtoolをCopilotが呼び出すと、WhiteboardAppに確認ダイアログが表示されます。
内容を確認して承認または拒否してください。Copilot側のtool実行確認とWhiteboardApp側の確認が
両方表示される場合があります。

## 7. 操作例

### 現在ページを画像として確認する

```text
Whiteboard MCPで現在ページのPNGを取得し、描かれている内容を説明してください。
```

### 新しいページへ図を作成する

```text
Whiteboard MCPを使って新しいページを作り、3段階の処理フロー図を配置してください。
変更前に実行内容を説明してください。
```

### Undoする

```text
Whiteboard MCPを使って直前の変更をUndoしてください。
```

図反映で指定できる要素やパラメーターの詳細は、
`docs/mcp_diagram_capabilities.md` を参照してください。

<div class="page-break"></div>

## 8. GitHub Copilot CLIから利用する

WhiteboardAppを通常起動してから、Copilot CLIへHTTP Serverを登録します。

```bash
copilot mcp add \
  --transport http \
  --tools '*' \
  whiteboard \
  http://127.0.0.1:8765/mcp
```

`copilot` を起動し、対話画面で `/mcp` を実行して `whiteboard` の接続状態を確認します。

CLIの利用者共通設定は `~/.copilot/mcp-config.json` に保存されます。手動設定する場合は
次の形式です。

```json
{
  "mcpServers": {
    "whiteboard": {
      "type": "http",
      "url": "http://127.0.0.1:8765/mcp",
      "tools": ["*"]
    }
  }
}
```

Visual Studio Codeの `.vscode/mcp.json` はトップレベルが `servers`、Copilot CLIの設定は
`mcpServers` である点に注意してください。

## 9. 標準入出力接続を使う場合

HTTP接続を使えない場合は、Visual Studio CodeからWhiteboardAppを `--mcp` 付きで起動できます。

```json
{
  "servers": {
    "whiteboard": {
      "type": "stdio",
      "command": "/Users/sugita/src/qt/tools/whiteboard/whiteboard-app-qtmcpserver/build/WhiteboardApp.app/Contents/MacOS/WhiteboardApp",
      "args": ["--mcp"]
    }
  }
}
```

動作比較では、Codexと同じ条件にできる通常起動とHTTP接続を優先してください。

## 10. トラブルシューティング

### `whiteboard` がtool一覧に表示されない

1. WhiteboardAppが起動しているか確認する。
2. `mcp.json` のURLが `http://127.0.0.1:8765/mcp` か確認する。
3. `mcp.json` 上部の状態を確認し、停止中なら `Start` を押す。
4. Copilot Chatが `Agent` モードか確認する。
5. コマンドパレットからMCP Serverの再起動または一覧表示を行う。
6. Copilot Business/Enterpriseの場合、組織のMCPポリシーを確認する。

### WhiteboardAppにポートエラーが表示される

`lsof` で8765番ポートの使用プロセスを確認します。別のWhiteboardAppが起動している場合は、
検証対象を1つに絞ってから再起動してください。

### 接続先が見つからない

VS CodeをRemote SSH、Dev Container、Codespacesで使用している場合、`127.0.0.1` はMacではなく
リモート環境を指すことがあります。WhiteboardAppとCopilot拡張を同じMac上で実行してください。

### 変更が反映されない

WhiteboardApp側の確認ダイアログが待機していないか確認してください。拒否した操作は反映されません。
ページがロックされている場合も変更操作は拒否されます。

## 11. GitHub.com上のCopilot coding agentについて

GitHub.com上で実行されるCopilot coding agentはクラウド環境で動作するため、このMacの
loopback endpointへは接続できません。本手順の対象は、Mac上のVisual Studio Codeと
GitHub Copilot CLIです。

このMCP Serverはloopback限定で、外部公開用の認証機能を持ちません。動作確認のために
インターネットへ直接公開しないでください。

## 12. 参考資料

- [GitHub Copilot ChatでMCP Serverを使う](https://docs.github.com/en/copilot/how-tos/provide-context/use-mcp-in-your-ide/extend-copilot-chat-with-mcp?tool=vscode)
- [Visual Studio CodeでMCP Serverを追加・管理する](https://code.visualstudio.com/docs/agent-customization/mcp-servers)
- [Visual Studio Code MCP configuration reference](https://code.visualstudio.com/docs/agents/reference/mcp-configuration)
- [GitHub Copilot CLIへMCP Serverを追加する](https://docs.github.com/en/copilot/how-tos/copilot-cli/customize-copilot/add-mcp-servers)
