# 付録 B. 生成物の考え方

Qt では、C++ ソース以外に生成物が必要になることがあります。

| 入力 | 生成物 | 使い方 |
|---|---|---|
| `Q_OBJECT` を含む `.h` / `.cpp` | `moc_*.cpp` / `*.moc` | 利用側 `.cpp` から include |
| `.ui` | `ui_*.h` | 利用側 `.cpp` から include |
| `.qrc` | `qrc_*.cpp` | 利用側 `.cpp` から include |
| `.ts` | `.qm` | `lrelease` で生成し、`.qrc` に入れる |

## moc placeholder

icpp は、`moc_*.cpp` / `*.moc` がまだ存在しない場合に、空の placeholder を作って include エラーを避けることがあります。

これは初回のつまずきを減らす補助です。本物の meta-object 情報ではないため、`Q_OBJECT`、slot、signal、`Q_PROPERTY` を使う場合は必ず `.gen` で本物を生成します。

`qrc_*.cpp`、`ui_*.h`、`.qm` には空 placeholder を作りません。これらは `.gen` または `lrelease` で本物を生成します。
