# NEXT

このファイルを読む、または更新する前に、`~/AGENTS.next.md` を読み、その指示に従うこと。
`~/AGENTS.next.md` が存在しない場合は、このファイルを整理・削除せず、利用者に確認すること。
このルールは、ユーザーから明示的な指示がない限り変更しない。

## 現在の状態

- `property_editor` の install 先を `icpp` の prefix に自動追従するようにした
- `icpp` の `.inspect` が使う `lib/` と `include/` への install を追加した
- macOS の install 済み `property_editor_demo` が `@rpath/property_editor.dylib` を解決できない問題を、demo 側の `INSTALL_RPATH` 追加で修正した
- ドキュメントは `USER_GUIDE.md` に統一し、`USER_GUILD.md` は廃止した
- release version は root の `VERSION` file で管理するようにした
- 配布 zip は `release/propertyeditor-<version>-source.zip` で管理するようにした
- `release/propertyeditor-1.0.0-source.zip` を作成し、`unzip -t` は OK、SHA-256 は `cd6c0a4a202680e8e825bb7f8ceac68d877d2fb18dda12d62fbe4916b9ad3758`
- `~/Sites/tools/propertyeditor/` へ `propertyeditor-1.0.0-source.zip` と `USER_GUIDE.md` をコピー済み

## 次にやること

- `/usr/local` など実運用 prefix へ再 install し、`property_editor_demo` と `icpp` の `.inspect` を実機で確認する

## 未完了

- `sudo cmake --install build` 後の `/usr/local/bin/property_editor_demo` 実行確認は、この作業では未実施
- `icpp` 側から `.inspect` を実行した統合確認は未実施

## 触るファイル

- `CMakeLists.txt`
- `VERSION`
- `README.md`
- `USER_GUIDE.md`
- `scripts/create_release_zip.sh`
- `release/README.md`

## 注意

- `property_editor` は `icpp` と同じ install prefix に入れる前提を維持する
- `USER_GUIDE.md` を正式な利用ガイドとし、配布物にもこちらを入れる
- macOS では demo 実行ファイル側の `INSTALL_RPATH` を消さない
- 配布 zip は `release/` 配下で管理し、zip top directory は `propertyeditor-<version>/` に揃える

## 関連ファイル

- `DECISIONS.md`
- `backlog.md`
- `README.md`
- `USER_GUIDE.md`
- `CMakeLists.txt`
- `VERSION`
