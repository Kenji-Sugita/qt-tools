# Qt Creator MCP 検証メモ

この記事素材は、Codex から Qt Creator MCP Server を使って Qt Creator を操作した検証結果をまとめたもの。

記事本文は別途作成する前提で、ここでは事実、手順、得られた知見を記録する。

## 検証日

2026-05-10

## 検証環境

- macOS
- Qt Creator
- Qt Creator MCP Server
- Codex
- Qt Widgets プロジェクト

Codex 側の MCP 登録:

```text
qtcreator  /Users/sugita/.local/bin/mcp-proxy  http://127.0.0.1:3001/sse
```

Qt Creator 側では `Tools -> MCP Server -> About MCP Server` で次を確認した。

```text
MCP Server is running
port 3001
```

通常ターミナルからは次で SSE endpoint に接続できた。

```sh
curl -i --max-time 3 http://127.0.0.1:3001/sse
```

結果:

```text
HTTP/1.1 200 OK
Content-Type: text/event-stream

event: endpoint
data: /sse
```

`curl` は timeout したが、SSE は接続を開いたままにするため、これは異常ではない。

## 接続トラブルと復旧

最初、Qt Creator 側では MCP Server が running と表示されていたが、Codex からの MCP tool 呼び出しは空応答になった。

症状:

- `list_projects` が空
- `list_open_files` が空
- `get_current_project` が空
- `open_file` も成功/失敗の情報が取れない

通常ターミナルでは `curl http://127.0.0.1:3001/sse` が成功したため、Qt Creator 側の server は動いていた。

復旧手順:

```sh
codex mcp remove qtcreator
codex mcp add qtcreator -- /Users/sugita/.local/bin/mcp-proxy http://127.0.0.1:3001/sse
codex mcp list
```

その後、Codex を再起動した。

再起動後は、次が正常に返った。

```text
list_projects
list_open_files
get_current_session
get_current_project
```

確認できた値:

```text
session: default
project: samegame3
openFiles: [...]
```

結論:

- Qt Creator MCP Server が途中で再起動・切断された場合、Codex 側の MCP 接続/proxy 状態が古くなることがある。
- MCP 登録を作り直し、Codex を再起動すると復旧した。

## MCP で確認できた基本操作

以下は実際に動作確認できた。

```text
list_projects
get_current_project
list_open_files
open_file
file_plain_text
build
get_build_status
list_issues
save_file
close_file
```

`open_file` でファイルを開き、`file_plain_text` で内容取得できた。

例:

```text
verification/qt-generated/README.md
```

`get_build_status` は次のような状態を返した。

```text
Building: 0%
Status: Not building
```

## ビルドエラー取得

Qt Creator のアクティブプロジェクトを `colorwidget` に変更し、C++ 側に意図的な typo を入れた。

エラー行:

```cpp
colorFrame->setAutoFillBackgroundx(true);
```

Qt Creator MCP で `build` した後、`list_issues` で次のエラーを取得できた。

```text
No member named 'setAutoFillBackgroundx' in 'QFrame'
did you mean 'setAutoFillBackground'?
```

該当ファイル:

```text
/Users/sugita/events/Qt_Seminer_Widget_slide_and_sample/examples/colorwidget_step2/colorwidget.cpp:10
```

確認できたこと:

- Qt Creator MCP は C++ のビルドエラーを取得できる
- エラー位置、ファイル名、候補修正を取得できる
- `list_issues` は Build Issues / TaskWindow 系の情報を見ている

## QML 実行時エラーについて

QML に実行時エラーを入れて実行したが、`list_issues` では取得できなかった。

分かったこと:

- QML runtime error は Application Output に出る
- 現在の MCP tool には Application Output を取得する tool がない
- `list_issues` は QML runtime error を対象にしていない

結論:

```text
Qt Creator MCP の list_issues は、現状ではビルドエラー確認用と考えるのがよい。
```

QML runtime error まで扱うには、MCP Server 側に次のような tool が必要。

```text
application_output
run_output
last_run_output
clear_application_output
```

## Codex で修正して Qt Creator に反映する検証

Codex 側で実ファイルを修正した。

修正前:

```cpp
colorFrame->setAutoFillBackgroundx(true);
```

修正後:

```cpp
colorFrame->setAutoFillBackground(true);
```

その直後、Qt Creator MCP の `file_plain_text` は古い内容を返すことがあった。

理由:

- Qt Creator が開いている editor buffer を保持している
- 外部変更が即座に読み直されない場合がある

対処:

```text
close_file
open_file
file_plain_text
```

この手順で Qt Creator 側にも修正済み内容が反映された。

確認できたこと:

- Codex が実ファイルを修正できる
- Qt Creator MCP で再読み込みできる
- `save_file` できる
- `build` できる

## list_issues の注意点

修正後に再ビルドしても、`list_issues` が古い issue を保持することがあった。

観察:

- 実ファイルは修正済み
- Qt Creator MCP の `file_plain_text` も修正済み
- `build` は完了
- しかし `list_issues` は過去の `setAutoFillBackgroundx` エラーを返した

したがって、自動判定では `list_issues` だけに頼らない。

併用する確認:

- 実ファイルに古い文字列が残っていない
- `file_plain_text` が修正後の内容を返す
- `build` が完了する
- `list_issues` に対象プロジェクトの新規エラーが増えていない

## 専用検証プロジェクト

既存教材プロジェクトを壊さないよう、Qt Creator MCP 検証専用の最小プロジェクトを作成した。

場所:

```text
verification/qtcreator-mcp/project/
```

構成:

```text
CMakeLists.txt
main.cpp
widget.h
widget.cpp
README.md
```

プロジェクト名:

```text
IcppQtCreatorMcpCheck
```

Qt Creator にプロジェクトとして開かせるには、次を実行した。

```sh
open -a "Qt Creator" verification/qtcreator-mcp/project/CMakeLists.txt
```

初回は Qt Creator 側で Configure Project / Kit 選択が必要だった。

Configure 後、MCP で次が返った。

```text
project: IcppQtCreatorMcpCheck
build config: Debug
```

この専用プロジェクトでも同じ roundtrip を確認した。

1. 正常ビルド
2. `widget.cpp` に typo を入れる
3. `file_plain_text` で Qt Creator に反映されたことを確認
4. `build`
5. `list_issues` で検証用プロジェクトの C++ ビルドエラーを取得
6. Codex が修正
7. `close_file` / `open_file` で Qt Creator に再反映
8. 再ビルド

取得できた検証用プロジェクトのエラー:

```text
/Users/sugita/src/tools/icpp/verification/qtcreator-mcp/project/widget.cpp:12
No member named 'setAutoFillBackgroundx' in 'QFrame'
did you mean 'setAutoFillBackground'?
```

## 記事にできるポイント

- Qt Creator MCP は、Qt Creator の現在のプロジェクトを外部から操作できる。
- Codex から Qt Creator の build / issues / open files / file text を扱える。
- C++ ビルドエラーを Codex が読み、修正候補を反映する流れが作れる。
- Qt Creator の editor buffer と実ファイルの同期には注意が必要。
- `list_issues` は古い issue を保持する場合がある。
- QML runtime error / Application Output は現時点では取れない。
- 専用検証プロジェクトを用意すると、既存作業を壊さずに MCP の動作確認ができる。

## 簡単な結論

Qt Creator MCP は、現時点でも C++ のビルドエラー修正支援には十分使える。

一方で、次は制約として明記した方がよい。

- Codex 側の MCP 接続は再起動で復旧が必要になる場合がある
- 初回の CMake project configure は Qt Creator 側で手動
- Application Output は取れない
- `list_issues` は古い issue を保持することがある

