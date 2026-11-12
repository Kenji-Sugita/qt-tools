件名: `qmlprop` 更新のお知らせ: Qt 6 向け整理と型調査機能の拡充

本文:

`qmlprop` を更新しました。今回の更新では、Qt 6 向けに実装を整理しつつ、QML 型を調べるための一覧機能と表示情報を強化しています。

主な更新点は次の通りです。

- Qt 5 向けコードを削除し、Qt 6 前提の構成へ整理
- qmake ベースの `.pro` から CMake ベースのビルドへ移行
- private header 依存を削除し、公開 API ベースの実装へ整理
- 型レポートに所属モジュール、型フラグ、デフォルトプロパティー、宣言元クラスの表示を追加
- `--show-attached` で Attached properties を表示可能
- `--show-declared-in` で各メンバーの宣言元クラスを表示可能
- `--all-matches` で同名型をモジュールごとに比較可能
- `--list-types` / `--list-modules` / `--module-index` で型とモジュールの索引を確認可能
- 端末接続時は既定でページャー経由にし、長い出力を読みやすく改善

基本的な使い方の例です。

```text
$ qmlprop Rectangle
$ qmlprop --all Item
$ qmlprop --show-attached Item
$ qmlprop --show-declared-in Item
```

一覧系の使い方の例です。

```text
$ qmlprop --list-types
$ qmlprop --list-modules
$ qmlprop --module-index QtQuick.Controls
```

同名型の切り分けや生成失敗の確認には、次のような使い方を想定しています。

```text
$ qmlprop --all-matches Button
$ qmlprop --debug FolderDialog
$ qmlprop -dd FolderDialog
```

操作方法と出力の見方は `USER_GUIDE.md` にまとめています。

QML 型の API 調査、継承メンバーの確認、Attached properties の確認、モジュールごとの型切り分けに活用してください。
