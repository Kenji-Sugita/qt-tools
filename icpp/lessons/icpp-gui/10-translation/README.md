# 10 翻訳を表示する

## このレッスンで学ぶこと

`lupdate`、Qt Linguist による `.ts` 編集、`lrelease`、`.qrc` を組み合わせて、翻訳された文字列をラベルに表示します。

## 操作

```text
icpp[qtcling]> .e widget.h
# 翻訳対象のラベルを持つ Widget クラスを書いて保存する
icpp[qtcling]> .e widget.cpp
# tr("Hello") と翻訳ファイル読み込みの実装を書いて保存する
icpp[qtcling]> .! lupdate widget.cpp -ts app_ja.ts
icpp[qtcling]> .linguist app_ja.ts
# Qt Linguist で 'Hello' を 'こんにちは' に翻訳する
# Qt Linguist で app_ja.ts を保存する
icpp[qtcling]> .! lrelease app_ja.ts
icpp[qtcling]> .qrc translations.qrc
# リソースに app_ja.qm が入っていることを確認して閉じる
icpp[qtcling]> .gen
icpp[qtcling]> .e widget.cpp
# qrc_translations.cpp のインクルード行のコメントを外す
# 保存して icpp に戻る
icpp[qtcling]> static auto w = go();
icpp[qtcling]> w->show();
icpp[qtcling]> w->raise();
icpp[qtcling]> .clean
```

最初は `app_ja.ts` がない状態から始めます。`lupdate` で作成し、Linguist で翻訳してから `lrelease` で `app_ja.qm` を生成します。

## 期待される結果

ウィンドウに翻訳後の `こんにちは` が表示されます。

## ポイント

`.gen` は `lrelease` を実行しません。翻訳では先に `.! lrelease app_ja.ts` を実行します。

`.linguist` は Qt Linguist を開くとすぐ icpp に戻ります。翻訳を編集したら、Qt Linguist 側で保存してから `.! lrelease app_ja.ts` を実行してください。

`.qrc` もエディタを開く操作です。`translations.qrc` に `app_ja.qm` が含まれていることを確認したら保存して閉じ、icpp に戻って `.gen` を実行します。

`lupdate` と `lrelease` は Qt 標準コマンドをそのまま使います。icpp 専用の短縮形は用意せず、オプションが見えるように `.! lupdate ...`、`.! lrelease ...` と明示します。
