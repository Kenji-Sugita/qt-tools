# qmlprop

QML 型のプロパティー/メソッド/列挙子を表示するコマンドラインプログラムです。

## 主な変更点

- Qt 5 向けのコードを削除
- Qt 6 向けに整理し、Qt 6.11.0 で使う前提の構成に更新
- qmake ベースの `.pro` から CMake ベースへ移行
- private header 依存を削除
- プライベートヘッダー依存を削除
- 巨大な固定 QML テンプレートを廃止し、候補モジュールを順番に試す方式へ変更
- `main.cpp` に集中していた責務を分割
- 型レポートへ、所属モジュール/型フラグ/デフォルトプロパティー/宣言元クラスの表
  示を追加
- `--list-types` に、生成可否や singleton/attached provider のフラグ表示を追加

## ビルド

```bash
cmake -S . -B build
cmake --build build
```

Qt 6 の共有ライブラリー構成を前提にしています。追加の QML モジュールを調べる場合
は、そのモジュールが Qt 側にインストールされている必要があります。

## 使い方

```bash
qmlprop Rectangle
qmlprop --all Item
qmlprop --debug FolderDialog
qmlprop --show-attached Item
qmlprop --all-matches Button
qmlprop --list-types
qmlprop --list-modules
qmlprop --module-index QtQuick.Controls
```

## 表示内容

通常の型レポートでは、次の情報を表示します。

- 所属モジュール
- 型フラグ(例: `creatable`, `singleton`, `attached provider`)
- C++ クラス階層
- デフォルトプロパティー/デフォルトメソッド
- プロパティー値
- プロパティー属性(`readonly`, `final`, `constant`, `required`, `bindable`,
  `default`, `deferred`, `list`, `object`)
- 各プロパティー/メソッド/列挙子の宣言元クラス
- 列挙子の数値

## 補足

- 型によっては、利用可能な QML モジュールが環境に入っていないと生成できません。
- 同名型が複数モジュールに存在する場合は、通常は最初に生成に成功した import 構成を優先
  して所属モジュールを推定します。
- `--all-matches` を付けると、同名型をモジュールごとに順番に表示します。
- 修飾名も使えます。例: `T.Control`, `QLabP.FolderDialog`
- `--list-types` は `qmldir` と `*.qmltypes` を走査し、値型/シーケンス型など
  `QObject` 系ではない型名を除外します。
- `--list-modules` は、現在の実行環境で見えている QML モジュール URI を一覧表示します。
- `--module-index <module>` は、指定モジュールの型を 1 行 1 型の表で表示します。
  これは QML 型の索引です。C++ API 中心のモジュールでは、表示件数が少ないことがあります。
- `--show-attached` を付けた場合だけ `Attached:` セクションを表示します。

追加のオプション:

- `--show-declared-in` `[declared in ...]` サフィックスをプロパティー/メソッド/列挙子/デフォルトメンバーに表示します。


## ページャー

標準出力がターミナルに接続されている場合、qmlprop は通常出力を既定でページャー経由にします。
これにより、長い型レポートでも `less` へ明示的にパイプしなくても読みやすくなります。

ページャーを無効にして標準出力へ直接書き出したい場合は、`--no-pager` を使ってください。
出力をリダイレクトしている場合や、別のコマンドへパイプしている場合は、自動的に標準出力へ直接書き出します。
