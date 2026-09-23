# DECISIONS

## 採用済み

### 1. `icpp` と同じ install prefix を既定にする

- `property_editor` は `icpp` の `.inspect` からの利用を前提にし、`CMAKE_INSTALL_PREFIX` 未指定時は `PATH` 上の `icpp` の install prefix を既定値に使う
- `icpp/bin/icpp` から見て `../lib/property_editor.*` と `../include/*.h` を自動検出できる配置を維持するため

### 2. release version は root の `VERSION` file で管理する

- source code release の版番号は root の `VERSION` file に置く
- `CMakeLists.txt` と配布 zip 名が同じ version を参照するようにして、版管理を 1 か所に集約するため

### 3. 配布 zip は `release/` 配下の source code release として扱う

- 配布 zip は `release/propertyeditor-<version>-source.zip` の命名規則で管理する
- zip top directory も `propertyeditor-<version>/` に揃える
- 作業用 `build/` や `install/` と配布物を分離し、`cling` と同じ release 管理に寄せるため

### 4. 利用ガイドは `USER_GUIDE.md` に統一する

- 正式な利用ガイドは `USER_GUIDE.md` とし、配布物にもこちらを含める
- `USER_GUILD.md` との二重管理を避け、参照先を 1 つに固定するため

## 不採用

### 1. `property_editor` を `icpp` と別 prefix に入れる運用

- 毎回 `ICPP_PROPERTY_EDITOR_LIB` と `ICPP_PROPERTY_EDITOR_INCLUDE` を手動設定する前提の運用
- `icpp` 側の相対探索を活かせず、利用手順が複雑になるため不採用
- `icpp` 側の探索仕様が変わる場合のみ再検討する

## まだ決めていないこと

- なし

## 判断基準

- `icpp` の `.inspect` が追加設定なしで動くことを優先する
- install 後の実行ファイルが OS 標準の探索規則だけで必要ライブラリを見つけられることを優先する
- release version と配布 zip 名がずれないことを優先する

## 継続する仕様・運用制約

- `property_editor` の install では `lib/` と `include/` の配置を `icpp` の相対探索に合わせる
- macOS の `property_editor_demo` では `@executable_path/../lib` を含む `INSTALL_RPATH` を維持する
- 配布 zip には `build/`、`install/`、`.DS_Store`、一時ファイルを含めない
