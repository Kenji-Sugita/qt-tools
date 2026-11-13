# Codex CLI から Qt Creator MCP を操作する

この手順では、Codex CLI から Qt Creator MCP Server を使って Qt Creator を操作する方法をまとめます。

この文書もQt Creator 19.0.0と旧HTTP+SSE接続を前提とした手順例です。同梱の `qtcreator-mcp-install.md` に記載した版と確認範囲を参照してください。現行版は [Qt Creator公式資料](https://doc.qt.io/qtcreator/creator-how-to-mcp-server.html) で確認します。現行版の設定や実機接続を今回検証したものではありません。

## 前提

Qt Creator MCP Server が起動していることを確認します。

    Tools
    -> MCP Server
    -> About MCP Server

Codex CLI に `qtcreator` MCP server が登録されていることを確認します。

    codex mcp list

`enabled` は設定上有効であることを示します。実際の接続・初期化と利用可能なtool一覧を確認し、読み取り操作が成功することを確かめます。ビルドや実行の前には対象プロジェクトと構成も確認します。

## 基本的な操作

Codex には tool 名を直接指定しなくても、自然文で依頼できます。

### プロジェクト一覧を確認する

    プロジェクトは何がありますか。

明示的に tool 名を指定する場合:

    Qt Creator MCP で list_projects を実行してください。

### 現在のプロジェクトを確認する

    現在のプロジェクトは何ですか。

### 現在のビルド構成を確認する

    現在のビルド構成は何ですか。

### ビルドする

    ビルドしてください。

### Issues を確認する

    Issues を確認してください。

### ビルドして Issues を確認する

    ビルドして Issues を確認してください。

## 応用操作

### ビルド構成を切り替える

利用可能なビルド構成を確認します。

    ビルド構成は何がありますか。

指定したビルド構成に切り替えます。

    Debug 構成に切り替えてください。

または:

    Release 構成に切り替えてください。

### 実行する

    実行してください。

### デバッグを開始する

    デバッグを開始してください。

### Issues の原因を説明してもらう

    Issues のエラーと警告を確認して、原因を説明してください。

### 修正して再ビルドする

    Issues の内容を確認して、必要な修正を行ってから再ビルドしてください。

### Qt Creator でファイルを開く

    Qt Creator で src/main.cpp を開いてください。

### 開いているファイルを確認する

    開いているファイルは何ですか。

### セッションを確認する

    Qt Creator のセッション一覧を確認してください。

### セッションを切り替える

    default セッションを読み込んでください。

## 操作一覧

| やりたいこと | Codex への依頼例 | 対応する MCP tool |
|---|---|---|
| プロジェクト一覧を確認する | プロジェクトは何がありますか。 | `list_projects` |
| 現在のプロジェクトを確認する | 現在のプロジェクトは何ですか。 | `get_current_project` |
| ビルド構成一覧を確認する | ビルド構成は何がありますか。 | `list_build_configs` |
| 現在のビルド構成を確認する | 現在のビルド構成は何ですか。 | `get_current_build_config` |
| ビルド構成を切り替える | Debug 構成に切り替えてください。 | `switch_build_config` |
| ビルドする | ビルドしてください。 | `build` |
| 実行する | 実行してください。 | `run_project` |
| デバッグを開始する | デバッグを開始してください。 | `debug` |
| Issues を確認する | Issues を確認してください。 | `list_issues` |
| ビルド状態を確認する | ビルド状態を確認してください。 | `get_build_status` |
| ファイルを開く | Qt Creator で src/main.cpp を開いてください。 | `open_file` |
| 開いているファイルを確認する | 開いているファイルは何ですか。 | `list_open_files` |
| セッション一覧を確認する | セッション一覧を確認してください。 | `list_sessions` |
| セッションを読み込む | default セッションを読み込んでください。 | `load_session` |
| セッションを保存する | 現在のセッションを保存してください。 | `save_session` |

## 明示的に tool 名を指定したい場合

通常は自然文で十分ですが、接続確認やトラブルシュートでは tool 名を指定すると意図が明確になります。

例:

    Qt Creator MCP で list_projects を実行してください。
    Qt Creator MCP で get_current_project を実行してください。
    Qt Creator MCP で list_issues を実行してください。

## 注意点

Qt Creator MCP Server は Qt Creator が起動している間だけ使えます。

Codex CLI に MCP server を追加した直後は、Codex CLI の再起動が必要です。

ビルドや実行は、Qt Creator 側で現在選択されているプロジェクトとビルド構成に対して行われます。

エラーを修正する場合は、Issues の内容と該当ソースコードを確認してから修正します。
