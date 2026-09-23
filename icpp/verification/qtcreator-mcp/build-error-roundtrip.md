# ビルドエラー修正 roundtrip

Qt Creator MCP を使い、Codex からファイル修正が Qt Creator に反映されることを確認する。

## 対象

`project/` の検証用 CMake プロジェクトを使う。

対象ファイル:

```text
verification/qtcreator-mcp/project/widget.cpp
```

初回は Qt Creator 側で `project/CMakeLists.txt` を開き、Kit / build config を設定する必要がある。

## 手順

1. Qt Creator MCP で現在のプロジェクトと build config を確認する。

```text
get_current_project
get_current_build_config
```

2. 対象ファイルへ意図的に typo を入れる。

```cpp
colorFrame->setAutoFillBackgroundx(true);
```

3. Qt Creator MCP でファイルを開き直し、Qt Creator 側のバッファに反映させる。

```text
close_file
open_file
file_plain_text
save_file
```

4. Qt Creator MCP でビルドする。

```text
build
get_build_status
list_issues
```

期待するエラー:

```text
No member named 'setAutoFillBackgroundx' in 'QFrame'
did you mean 'setAutoFillBackground'?
```

5. Codex 側で typo を修正する。

```cpp
colorFrame->setAutoFillBackground(true);
```

6. Qt Creator MCP で再読み込みして、修正後の内容を確認する。

```text
close_file
open_file
file_plain_text
save_file
```

7. 再ビルドする。

```text
build
get_build_status
list_issues
```

## 確認済みの挙動

- `project/` を Qt Creator で Configure Project した後、MCP の `build` でビルドできる
- 検証用プロジェクトの `widget.cpp` に入れた typo を `list_issues` で取得できる
- Codex が実ファイルを修正できる
- Qt Creator MCP の `file_plain_text` は、開いているバッファが古い場合がある
- `close_file` / `open_file` で Qt Creator 側に外部変更を反映できる
- `save_file` と `build` は MCP 経由で実行できる
- C++ ビルドエラーは `list_issues` で取得できる
- `list_issues` は修正後も古い issue を保持する場合がある

2026-05-10 の確認では、修正後も `list_issues` には過去の `setAutoFillBackgroundx` エラーが残った。ただし、`file_plain_text` と実ファイルの両方で修正済み内容を確認でき、再ビルドも完了した。

## 残る注意点

修正後の成功判定は `list_issues` だけに頼らない。実ファイルと Qt Creator 側の `file_plain_text` に古いエラー文字列が残っていないことも確認する。
