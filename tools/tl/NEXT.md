# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 次にやること

- Google Cloud 側で Service Account JSON キーを用意し、`GOOGLE_APPLICATION_CREDENTIALS` を設定して実API疎通を確認する。
- 確認コマンド: `cd /Users/sugita/src/qt/tools/tl && ./build/tl "今日は会議があります。"`

## 未完了

- 実翻訳は未確認。現在の環境では `GOOGLE_APPLICATION_CREDENTIALS` が未設定で、`./build/tl "test"` は `tl: GOOGLE_APPLICATION_CREDENTIALS is not set` になる。
- `build/` は今回の調査で生成した未追跡ビルド成果物。コミット対象にしない。

## 触るファイル

- `GOOGLE_FREE_SETUP.md`
- `USER_GUIDE.md`
- `src/googleapiclient.cpp`

## 注意

- `tl` 本体はコミット `c6c8e1b3e 翻訳 CLI ツールを追加` で追加済み。
- `cmake -S . -B build`、`cmake --build build`、`ctest --test-dir build --output-on-failure` は成功済み。
- `/opt/local/bin/openssl` は存在し、`OpenSSL 3.6.2 7 Apr 2026` が実行できる。
- Google Cloud Translation の設定手順は `GOOGLE_FREE_SETUP.md` に保存済み。
- 配布用 `tl.zip` は作成済み。トップは `tl/`、内容はソース、CMake、利用者向け文書、テストのみ。
- `USER_GUIDE.pdf` は `genpdf --format book --output USER_GUIDE.pdf USER_GUIDE.md` で作成済み。A4 4ページで、PNG レンダリング確認済み。
