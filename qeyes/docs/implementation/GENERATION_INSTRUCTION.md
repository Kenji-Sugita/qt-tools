# 生成指示文

この文書の指示は、空の `qeyes/` ディレクトリを作成し、その直下に `PRODUCT_DESIGN.md`、`UI_DESIGN.md`、`RENDERING_DESIGN.md`、`AGENTS.md`、`IMPLEMENTATION_TEMPLATE.md` を配置した状態を前提にする。

標準の生成指示文は次の通りです。

```text
PRODUCT_DESIGN.md、UI_DESIGN.md、RENDERING_DESIGN.md、AGENTS.md に従って、
CMakeLists.txt、main.cpp、qeyes.rc、assets/ を生成してください。
必要なら IMPLEMENTATION_TEMPLATE.md も参照してください。
```

更新時は次のように指示します。

```text
PRODUCT_DESIGN.md、UI_DESIGN.md、RENDERING_DESIGN.md、AGENTS.md の内容に合わせて
CMakeLists.txt、main.cpp、qeyes.rc、assets/ を更新してください。
必要なら IMPLEMENTATION_TEMPLATE.md も参照してください。
```
