# Qt Creator MCP Server のインストール確認

この手順は、記載されたmacOSの配置とQt Creator 19.0.0を前提とする旧版の手順例です。今回、同版で再実行した記録ではありません。現行版では画面・ポート設定が異なるため、利用する版の公式資料と実際の接続URLを確認します。

現行版の設定項目は [Qt Creator公式資料](https://doc.qt.io/qtcreator/creator-how-to-mcp-server.html) を参照してください。この旧手順例をそのまま現行版へ適用しないでください。

## 前提

Qt Creator は次の場所にあります。

    /usr/local/qt/Qt/Qt Creator.app

Qt Creator 19.0.0 には MCP Server plugin が含まれています。

    /usr/local/qt/Qt/Qt Creator.app/Contents/PlugIns/qtcreator/libmcpserver.dylib

## Qt Creator を起動

    open "/usr/local/qt/Qt/Qt Creator.app"

## MCP Server extension を有効化

Qt Creator の UI で次を実行します。

    Extensions
    -> MCP Server
    -> Active
    -> Restart Now

## MCP Server の状態確認

Qt Creator の UI で次を開きます。

    Tools
    -> MCP Server
    -> About MCP Server

次のように表示されれば正常です。

    MCP Server is running
    The MCP server is active on port 3001 and accepting connections.

## MCP Server URL

この旧手順例では次のURLを使います。接続時は実際の待ち受けアドレスとポートを確認してください。

    http://127.0.0.1:3001/sse

## ポートを変える場合

`3001` が使用中の場合は、Qt Creator 起動前に `MCP_SERVER_PORT` を指定します。

    MCP_SERVER_PORT=3002 "/usr/local/qt/Qt/Qt Creator.app/Contents/MacOS/Qt Creator"

この場合の URL は次です。

    http://127.0.0.1:3002/sse
