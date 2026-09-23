# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 現在の状態

- `propertyeditor` と同じインストール構成を `objectselector` に適用した。
- `CMakeLists.txt` は icpp のインストール先の自動検出、公開ヘッダのインストール、macOS の実行時ライブラリパスに対応している。
- 構成、ビルド、インストール、およびインストール済みデモの RPATH 確認は完了した。
- `VERSION` を唯一の版数源として、1.0.0 の source code release ZIP、書籍形式の `USER_GUIDE.pdf`、および配布先 `/Users/sugita/Sites/tools/objectselector` への配置を完了した。

## 次にやること

- GUI セッションを使える環境で `ctest --test-dir build --output-on-failure` を実行し、`objectpicker_test` を確認する。

## 未完了

- 非 GUI 環境では `objectpicker_test` が画面を作成できず `no screens available` で停止する。GUI セッションで再検証が必要。

## 触るファイル

- `CMakeLists.txt`
- `tests/objectpicker_test.cpp`（ヘッドレス実行対応を検討する場合）
- `VERSION`
- `release.sh`

## 注意

- `CMakeLists.txt` を変更する前に `~/AGENTS.cmake.md` を確認する。
- インストール先は `ICPP_PREFIX`、または PATH 上の `icpp` の親ディレクトリを既定値にする。`CMAKE_INSTALL_PREFIX` を明示した場合はそれを優先する。
- source code release は `./release.sh` で作成する。`release/objectselector-<VERSION>-source.zip`、`USER_GUIDE.pdf` は Git 管理し、配布先にも同名で配置する。

## 関連ファイル

- `CMakeLists.txt`
- `USER_GUIDE.md`
- `USER_GUIDE.pdf`
- `release.sh`
- `release/objectselector-1.0.0-source.zip`
- `tests/objectpicker_test.cpp`
