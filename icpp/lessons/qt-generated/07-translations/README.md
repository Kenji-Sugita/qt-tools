# 07 Translations

## このレッスンで学ぶこと

`.ts` を `lrelease` で `.qm` にし、`.qrc` 経由で `QTranslator` が読み込む流れを学びます。

翻訳 resource は、`.gen` だけでは完結しません。先に `lrelease` を明示的に実行します。

`lupdate` と `lrelease` は Qt 標準コマンドをそのまま使います。icpp 専用の短縮形は用意せず、オプションが見えるように `.! lupdate ...`、`.! lrelease ...` と明示します。

## 事前確認

このディレクトリに次のファイルがあります。

```text
app_ja.ts
translations.qrc
widget.cpp
```

`translations.qrc` は、`lrelease` で作られる `app_ja.qm` を resource に含めます。

## 操作

```text
icpp[qtcling]> .! lupdate widget.cpp -ts app_ja.ts
icpp[qtcling]> .linguist app_ja.ts
icpp[qtcling]> .! lrelease app_ja.ts
icpp[qtcling]> .qrc translations.qrc
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
icpp[qtcling]> .files
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
```

## 期待される結果

- `.! lrelease app_ja.ts` で `app_ja.qm` が生成されます
- `.gen` で `qrc_translations.cpp` が生成されます
- `go()` で翻訳後の `こんにちは` を表示する widget が作られます

## なぜそうするか

`.ts` は翻訳元データ、`.qm` は実行時に読むバイナリ翻訳ファイルです。`.qrc` は `.qm` を resource として取り込みます。

## よくある失敗

- `lrelease` を忘れると `.qm` がなく、resource に入れられません
- `lrelease` の前に `.gen` すると古い `.qm` または存在しない `.qm` を参照します
- `lupdate` は `--ts` ではなく `-ts` を使います
- 現時点の `.gen` は `lrelease` を自動実行しません
- `.lupdate` / `.lrelease` のような icpp 専用 alias はありません

## 発展

`app_ja.ts` の翻訳文を変更し、`lrelease`、`.gen`、`go()` の順に再確認してください。
