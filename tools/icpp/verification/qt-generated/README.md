# Qt 生成物検証

`QT_PATTERN_CHECKLIST.md` に沿って、icpp で Qt 生成物を含むコードを検証する作業場です。

## 構成

| ディレクトリ | 内容 |
|---|---|
| `common/` | 共通スクリプトと期待結果の書式 |
| `01-header-qobject/` | ヘッダー分離した `Q_OBJECT` |
| `02-ui-only/` | `.ui` だけを使う |
| `03-qobject-ui/` | `Q_OBJECT` と `.ui` |
| `04-qrc-only/` | `.qrc` だけを使う |
| `05-qobject-qrc/` | `Q_OBJECT` と `.qrc` |
| `06-qobject-ui-qrc/` | `Q_OBJECT`、`.ui`、`.qrc` |
| `07-translations/` | `.ts` / `.qm` / `.qrc` |

## 基本操作

各ディレクトリで次を実行します。

```sh
../common/run_case.sh
```

`input.icpp` を icpp に流し、`output.txt` を作ります。完全な GUI 操作の自動確認ではなく、生成物、登録順、定義一覧、起動可能性を確認するための半自動検証です。
