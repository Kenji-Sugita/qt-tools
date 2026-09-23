# 非対話検証の仕組み

`verification/qt-generated/` は、Qt 生成物まわりを非対話で流せる検証ケース集として作っている。

## 構成

```text
verification/qt-generated/
├── common/
│   ├── run_case.sh
│   └── clean_generated.sh
├── 01-header-qobject/
├── 02-ui-only/
├── 03-qobject-ui/
├── 04-qrc-only/
├── 05-qobject-qrc/
├── 06-qobject-ui-qrc/
└── 07-translations/
```

各ケースには主に次のファイルがある。

```text
input.icpp      icpp に流す入力
expected.txt    期待する生成物・出力・手動確認
*.cpp, *.h, .ui, .qrc, .ts  検証用ソース
```

## 1 ケースだけ実行する

各ケースのディレクトリで実行する。

```sh
../common/clean_generated.sh
../common/run_case.sh
```

`run_case.sh` は、現在のディレクトリの `input.icpp` を icpp に渡し、`output.txt` を作る。

```sh
icpp < input.icpp > output.txt 2>&1
```

`ICPP_BIN` が指定されていなければ、既定で次を使う。

```text
tools/icpp/build/icpp
```

Qt framework のライセンス確認で止まらないように、検証時は既定で次を付ける。

```sh
QTFRAMEWORK_BYPASS_LICENSE_CHECK=1
```

## 全ケースを実行する

```sh
for d in verification/qt-generated/[0-9][0-9]-*; do
  echo "== $d =="
  (cd "$d" && ../common/clean_generated.sh && ../common/run_case.sh)
done
```

## エラー確認

```sh
rg -n "error:|fatal error|Unknown command|Cannot|not found|failed|open failed|translation load failed|too many errors|Stack dump" \
  verification/qt-generated/*/output.txt
```

この検索で何も出なければ、非対話検証としては通過と判断する。

## 例: qrc only

`04-qrc-only/input.icpp` は次の流れを確認する。

```text
.i .
.add widget.cpp
.gen
.defs
.p resourceText()
.quit
```

確認していること:

- `.i .` で include path が通る
- `.add widget.cpp` で登録される
- `.gen` で `qrc_resources.cpp` が生成される
- `.defs` で定義が見える
- `.p resourceText()` で resource を実際に読める

## 自動検証の範囲

自動で確認すること:

- `moc` / `uic` / `rcc` / `lrelease` による生成物作成
- `.gen` 後の再評価
- `.defs` による定義確認
- `go()` の呼び出し
- resource の読み込み
- `.qm` 翻訳 resource の読み込み

自動では確認しないこと:

- GUI の目視
- ボタンクリック
- `.e` の外部エディタ操作
- クリップボード操作
- Qt Creator 連携

これらは手動確認として扱う。

