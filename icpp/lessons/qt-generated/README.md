# icpp Qt 生成物レッスン

このディレクトリは、icpp で `moc` / `uic` / `rcc` / `lrelease` を含む Qt コードを練習するための教材です。

`verification/qt-generated/` は検証用、こちらはユーザーが手で試す練習用です。

## 進め方

各 lesson のディレクトリへ移動し、`README.md` の手順を icpp で実行します。

```sh
cd lessons/qt-generated/01-header-qobject
../../../build/icpp
```

icpp の起動後は、基本的に次の流れで進めます。

```text
icpp[qtcling]> .add widget.cpp
icpp[qtcling]> .gen
icpp[qtcling]> .defs
```

## 学ぶこと

- `.h` / `.ui` / `.qrc` / `.ts` は編集できるが、自動登録されない
- `.cpp` は `.e <file.cpp>` または `.add <file.cpp>` で評価対象にする
- `.gen` で `moc` / `uic` / `rcc` を実行する
- `.autogen on` で `.r` 前に自動生成する
- `.defs` / `.p` で結果を確認する
- objectName の不一致を interpreter の診断と `.uiinfo` で見る

## 手順ファイル

各 lesson は `input.icpp` も持っています。README の操作をまとめて流すための参考ファイルです。

```sh
../../../build/icpp < input.icpp > output.txt 2>&1
```

ただし、表示用 widget を `show()` する lesson は GUI セッションが必要です。画面のない環境で非対話実行すると `Cannot create window: no screens available` になることがあります。

このディレクトリの主目的は自動テストではなく、手で操作して理解することです。自動検証は `verification/qt-generated/` を使います。
