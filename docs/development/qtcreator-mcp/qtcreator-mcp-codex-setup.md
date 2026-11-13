# Codex CLI で Qt Creator MCP Server を使う設定

この手順では、Qt Creator の SSE MCP Server を Codex CLI から使えるようにします。

この文書もQt Creator 19.0.0と旧HTTP+SSE接続を前提とした手順例です。同梱の `qtcreator-mcp-install.md` に記載した版と確認範囲を参照してください。現行版は [Qt Creator公式資料](https://doc.qt.io/qtcreator/creator-how-to-mcp-server.html) で確認します。現行版の設定や実機接続を今回検証したものではありません。

## 前提

Qt Creator MCP Server が起動していることを確認します。

    Tools
    -> MCP Server
    -> About MCP Server

この旧手順例で使うURLは次です。実際の待ち受けアドレスとポートを確認します。

    http://127.0.0.1:3001/sse

## mcp-proxy のインストール

Codex CLI から Qt Creator の SSE MCP Server に接続するために `mcp-proxy` を使います。

以下はMacPortsを使うmacOSでの導入例です。既に必要なツールがある場合はインストールを繰り返さず、所属組織で認められた導入方法を使います。旧HTTP+SSE接続に対してstdio変換を使う例であり、Streamable HTTPの直接接続とは区別します。

MacPorts で `pipx` をインストールします。

    sudo port selfupdate
    sudo port install pipx

root shell に入っている場合は、通常ユーザーに戻ります。

    exit

通常ユーザーで `mcp-proxy` をインストールします。

    pipx install mcp-proxy

`mcp-proxy` の場所を確認します。

    command -v mcp-proxy

PATH に出てこない場合は次を確認します。

    ~/.local/bin/mcp-proxy

## Codex CLI に MCP server を登録

`mcp-proxy` が PATH にある場合:

    codex mcp add qtcreator -- mcp-proxy http://127.0.0.1:3001/sse

PATH にない場合:

    codex mcp add qtcreator -- ~/.local/bin/mcp-proxy http://127.0.0.1:3001/sse

登録状態を確認します。

    codex mcp list

例:

    Name       Command                             Args                       Env  Cwd  Status   Auth
    qtcreator  /Users/<user>/.local/bin/mcp-proxy  http://127.0.0.1:3001/sse  -    -    enabled  Unsupported

## Codex CLI を再起動

MCP server を追加した後は、Codex CLI を一度終了して起動し直します。

    /quit

または Ctrl-D で終了します。

その後、再度起動します。

    codex

## 動作確認

Codex CLI 起動後、次のように依頼します。

    Qt Creator MCP で list_projects を実行してください。

正常に接続できていれば、Qt Creator に読み込まれているプロジェクト一覧が返ります。

説明用の例:

    Projects:
    - example-project-a
    - example-project-b

## よく使う依頼例

現在のプロジェクトを確認します。

    Qt Creator MCP で現在のプロジェクトを確認してください。

現在のビルド構成を確認します。

    Qt Creator MCP で現在のビルド構成を確認してください。

Qt Creator でビルドします。

    Qt Creator MCP で build を実行してください。

Issues を確認します。

    Qt Creator MCP で list_issues を実行してください。

ビルドして Issues を確認します。

    Qt Creator MCP でビルドして、Issues を確認してください。
