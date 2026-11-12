# Qt Creator MCP 検証用プロジェクト

Qt Creator MCP Server の build / issues / file reload を確認するための最小 Qt Widgets プロジェクト。

## 使い方

Qt Creator でこのファイルをプロジェクトとして開く。

```text
verification/qtcreator-mcp/project/CMakeLists.txt
```

Codex から確認する内容:

1. `get_current_project` が `IcppQtCreatorMcpCheck` を返す
2. `build` が成功する
3. `widget.cpp` に typo を入れる
4. `build` 後に `list_issues` で C++ ビルドエラーを取得する
5. Codex で typo を修正する
6. `close_file` / `open_file` / `file_plain_text` で Qt Creator 側に反映する
7. 再ビルドする

意図的に壊す行:

```cpp
colorFrame->setAutoFillBackground(true);
```

例:

```cpp
colorFrame->setAutoFillBackgroundx(true);
```

