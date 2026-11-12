<!--
genpdf --title 'qmlprop 利用ガイド' --autho='(株)SRA' qmlprop_user_guide.md
-->

# 1. qmlprop とは

`qmlprop` は、QML 型を実際に生成して、その型に見えている情報を表示するコマンドラインツールです。

主に次のような用途で使います。

- ある QML 型のプロパティー/メソッド/列挙子を調べる
- 継承元まで含めた API を確認する
- デフォルトプロパティー/デフォルトメソッドを確認する
- アタッチトプロパティーを確認する
- 同名型が複数ある場合に、どのモジュールの型かを切り分ける
- 型の生成失敗理由を調べる

---

# 2. できること

`qmlprop` は、対象の型を QML 上で生成し、`QObject`/`QMetaObject` 経由で次の情報を表示します。

- モジュール名
- 型フラグ
  - `creatable`
  - `uncreatable`
  - `singleton`
  - `attached provider`
  - `value type`(一覧/失敗診断向け)
- C++ クラス階層
- デフォルトプロパティー
- デフォルトメソッド
- プロパティー一覧
  - 型
  - 現在値
  - 属性
    - `readonly`
    - `final`
    - `constant`
    - `required`
    - `bindable`
    - `default`
    - `deferred`
    - `list`
    - `object`
    - `override`
    - `inherited`
- メソッド一覧
- 列挙子一覧
- 各メンバーの宣言元 C++ クラス
- Attached properties
- 同名型の全候補
- 生成失敗理由の分類

---

# 3. ビルド

```bash
cmake -S . -B build
cmake --build build
```

補足です。

- Qt 6 の開発環境が必要です。
- 調べたい QML モジュールが実行環境にインストールされている必要があります。
- 追加モジュールが無い場合、その型は生成できないことがあります。

---

# 4. 基本的な使い方

## 4.1 型を1つ調べる

```bash
qmlprop Rectangle
```

例:

```text
Qt 6.11.0
Type: Rectangle
Module: QtQuick
Type flags: creatable
Class: QQuickRectangle -> QQuickItem -> QObject
Default property: data

Properties:
    color: QColor = "#ffffffff"
    radius: double = 0
    ...
```

## 4.2 継承メンバーも含めて表示する

```bash
qmlprop --all Item
```

`--all` を付けない場合は、その型自身で宣言されたメンバー中心に表示します。`--all` を付けると、継承元のメンバーも含めて表示します。

## 4.3 デバッグ情報を表示する

```bash
qmlprop --debug FolderDialog
qmlprop -dd FolderDialog
```

`--debug` は繰り返し指定できます。

- `-d` / `--debug`: 選ばれた import プロファイルや生成に使った QML ソースを表示します。
- `-dd`: さらに詳しい失敗ログを表示します。

## 4.4 Attached properties を表示する

```bash
qmlprop --show-attached Item
```

## 4.5 宣言元クラス名も表示する

```bash
qmlprop --show-declared-in Item
```

このオプションを付けると、各プロパティー/メソッド/列挙子の末尾に `[declared in ...]` が付きます。

## 4.6 利用可能な型名を一覧表示する

```bash
qmlprop --list-types
```

一覧では、型ごとにモジュールとフラグも表示します。

## 4.7 利用可能なモジュールを一覧表示する

```bash
qmlprop --list-modules
```

一覧では、現在の実行環境で見えている QML モジュール URI を表示します。

## 4.8 モジュール全体の索引を表示する

```bash
qmlprop --module-index QtQuick.Controls
```

一覧では、指定モジュール内の型を 1 行 1 型で表示します。
主に次の列を表示します。

- `Type`: 型名
- `Base`: 基底クラス名
- `Flags`: 型フラグ
- `Props`: その型自身で宣言されたプロパティー数
- `Methods`: その型自身で見えている public/protected メソッド数
- `Enums`: その型自身で宣言された列挙子数
- `Default`: デフォルトプロパティー名
- `Status`: `ok` / `failed` / `no profile`

注意点:

- これは「そのモジュールで見えている QML 型」の索引です。C++ API が中心のモジュールでは、表示件数が 1 件だけ、またはかなり少ないことがあります。
- たとえば `QtNetwork` はこの環境では `NetworkInformation` だけが QML 型として見えており、索引も 1 行になります。
- `Status: no profile` は、型カタログ上はそのモジュールの型が見えているが、`qmlprop` 側にはそのモジュールを試す import プロファイルがまだ無いことを意味します。

---

# 5. コマンドラインオプション

## 5.1 一覧

| オプション | 意味 |
|---|---|
| `-a`, `--all` | 継承メンバーも表示します。 |
| `-l`, `--list-types` | 現在の実行環境で見えている importable な QML 型名を一覧表示します。 |
| `-m`, `--list-modules` | 現在の実行環境で見えている QML モジュール URI を一覧表示します。 |
| `--module-index <module>` | 指定した QML モジュールの型を 1 行 1 型の表で表示します。 |
| `-d`, `--debug` | デバッグ情報を表示します。繰り返し指定で詳細化します。 |
| `--show-attached` | Attached properties を表示します。 |
| `--show-declared-in` | 各メンバーの宣言元 C++ クラスを表示します。 |
| `--all-matches` | 同名型をモジュールごとにすべて表示します。 |
| `--no-pager` | pager を使わず標準出力へ直接書き出します。 |
| `-h`, `--help` | ヘルプを表示します。 |
| `-v`, `--version` | バージョンを表示します。 |

## 5.2 positional argument

```bash
qmlprop [options] <type>
```

`<type>` は QML 型名です。

例:

- `Rectangle`
- `Item`
- `FolderDialog`
- `Component`
- `T.Control`
- `QLabP.FolderDialog`

---

# 6. 出力の見方

## 6.1 先頭部分

通常の型レポートは、概ね次の順で表示されます。

1. `Qt 6.11.0` などの Qt バージョン
2. `Type:`
3. `Module:`
4. `Type flags:`
5. `Class:`
6. `Default property:` / `Default method:`
7. `Properties:`
8. `Attached:`
9. `Methods:`
10. `Enumerators:`

## 6.2 `Module:`

`Module:` には、実際に生成に成功した import プロファイルを優先して推定したモジュールが表示されます。

同名型が複数モジュールに存在する場合、単純な名前一致だけでなく、**実際に成功した import 構成**を優先して表示します。

## 6.3 `Type flags:`

代表的なフラグです。

- `creatable`: その型を通常の QML オブジェクトとして生成できる想定です。
- `uncreatable`: 型登録はあるが、そのままでは通常生成できません。
- `singleton`: singleton 型です。
- `attached provider`: attached type を持つ型です。
- `value type`: 値型です。`QObject` ベースではないため通常のオブジェクト生成対象ではありません。

## 6.4 `Class:`

実際に見えている C++ クラス階層です。QML 用の内部ラッパー型はなるべく除外し、実体のクラス階層が分かるようにしています。

## 6.5 `Properties:`

各プロパティー行では、次のような情報が表示されます。

```text
text: QString = "Hello"
width: double = 100
visible: bool = true
```

属性も必要に応じて表示されます。

```text
model: QObject* = null readonly
status: QQmlComponent::Status -- readonly
delegate: QQmlComponent* = null default object
```

## 6.6 `Methods:` / `Enumerators:`

メソッドはシグネチャ付きで表示されます。列挙子はキーと数値が表示されます。

---

# 7. 型の見つけ方

## 7.1 まずはそのまま型名を指定する

```bash
qmlprop Label
```

`qmlprop` は内部で複数の import プロファイルを順に試し、最初に生成できたものを表示します。

## 7.2 修飾名を使う

一部の型は修飾名で指定できます。

```bash
qmlprop T.Control
qmlprop QLabP.FolderDialog
```

現時点で特別扱いしている alias は主に次です。

- `T` → `QtQuick.Templates`
- `QLabP` → `Qt.labs.platform`

## 7.3 `--list-types` で候補を探す

```bash
qmlprop --list-types | grep Label
```

同名型が複数見つかることがあります。

---

# 8. 同名型が複数ある場合

## 8.1 既定動作

同名型が複数モジュールにある場合、`qmlprop` は複数の import プロファイルを順番に試し、**最初に生成に成功したもの**を表示します。

## 8.2 すべての候補を表示する

```bash
qmlprop --all-matches Label
```

例:

```text
== Match 1/7: Qt.labs.StyleKit::Label [creatable] ==
No import profile is available for module: Qt.labs.StyleKit

== Match 2/7: QtQuick.Controls.Basic::Label [creatable] ==
No import profile is available for module: QtQuick.Controls.Basic

== Match 3/7: QtQuick.Templates::Label [creatable] ==
Qt 6.11.0
Type: Label
Module: QtQuick.Templates
Type flags: creatable
Class: QQuickLabel -> QQuickText -> QQuickImplicitSizeItem -> QQuickItem -> QObject
...
```

ここで重要なのは、候補が表示されたからといって、**その候補を必ず試せているとは限らない**ことです。

- `No import profile is available for module: ...`
  - その型候補は型カタログ上に見つかった
  - ただし、現状の `qmlprop` にはそのモジュール用の import プロファイルが無い
  - そのため、その候補はスキップされた

- 型レポートが続く場合
  - その候補は実際に生成に成功した
  - `qmlprop` はその候補の通常レポートを表示する

注意点:

- 現在の `--all-matches` は、候補ごとの失敗診断を全部は表示しません。
- import プロファイルが無い候補は見えますが、**生成を試して失敗した候補は表示されずにスキップされる**実装です。
- そのため、`--all-matches` の出力件数は「型カタログ上の全候補数」と一致しないことがあります。

## 8.3 Qt Quick Controls の `Label` を見たい場合

多くの環境では次で確認できます。

```bash
qmlprop Label
qmlprop --debug Label
```

`--debug` を付けると、どの import プロファイルが選ばれたかが分かります。

```text
Selected profile: QtQuick.Controls
```

のように出れば、実際に表示しているのは `QtQuick.Controls` 側の `Label` です。

---

# 9. 生成失敗時の診断

型を生成できない場合は、単に `could not be instantiated` と出すだけでなく、理由分類も表示できます。

例:

```text
SomeType could not be instantiated.
Reason: required property not set
Details: Required property 'model' is not initialized.
```

## 9.1 主な失敗理由

| `Reason:` | 意味 |
|---|---|
| `required property not set` | required property が未設定のため生成に失敗しました。 |
| `value type` | 値型であり、`QObject` ベースの QML オブジェクトとして生成できません。 |
| `abstract type` | 抽象型のため直接生成できません。 |
| `attached-only type` | attached type としてのみ使う型です。 |
| `module import missing` | 試した import 構成では型名が解決できませんでした。必要なモジュールが不足している可能性があります。 |
| `dependent type missing` | 依存している別の型/モジュールが不足している可能性があります。 |
| `uncreatable type` | 型登録は見つかったが、通常の QML オブジェクトとしては生成不可です。 |
| `unknown` | 上記に分類できない失敗です。`--debug` で詳細を確認してください。 |

## 9.2 `--debug` との併用

```bash
qmlprop --debug Component
qmlprop -dd Component
```

詳しい失敗確認では、次の情報が役立ちます。

- 試した import プロファイル一覧
- 実際に生成に使った QML ソース
- `QQmlError` の内容
- `Reason:` と `Details:`

---

# 10. import プロファイルの考え方

`qmlprop` は、型名を見つけたら即終了するのではなく、複数の import 構成を順に試して生成を確認します。

代表的には次のようなプロファイルがあります。

- Base modules
  - `QtCore`
  - `QtQml`
  - `QtQuick`
- `QtQuick.Controls`
- `QtQuick.Templates`
- `QtQuick.Dialogs`
- `QtQuick.Pdf`
- `QtQuick.Shapes`
- `QtQuick.Layouts`
- `QtQuick.Window`
- `QtQuick3D`
- `QtLocation`
- `QtMultimedia`
- `QtWebEngine`
- `Qt.labs.platform`
- `Qt.labs.settings`
- そのほか対応済みモジュール

注意点です。

- `--list-types` では見える型でも、`qmlprop` 側に対応 import プロファイルが無い場合は直接試せません。
- 実装用/内部用のモジュール候補が見えることがあります。
- `--all-matches` は候補列挙に強い一方で、「見つかったが未対応」の候補も表示します。

## 10.1 pager について

標準出力が端末につながっている場合、通常の型レポートは既定で pager 経由で表示されます。

- 長い出力を端末上で読みやすくするための動作です。
- パイプやリダイレクト時は、自動で標準出力へ直接書き出します。
- pager を使いたくない場合は `--no-pager` を指定します。

```bash
qmlprop --no-pager Rectangle
qmlprop --list-types --no-pager
```

---

# 11. よく使う例

## 11.1 Qt Quick 基本型

```bash
qmlprop Rectangle
qmlprop Item
qmlprop --all Item
```

## 11.2 Controls 系

```bash
qmlprop Button
qmlprop Label
qmlprop --debug Label
```

## 11.3 Dialogs / labs.platform

```bash
qmlprop FolderDialog
qmlprop QLabP.FolderDialog
```

## 11.4 Templates alias

```bash
qmlprop T.Control
```

## 11.5 型一覧から探す

```bash
qmlprop --list-types | grep Dialog
qmlprop --list-types | grep Pdf
```

## 11.6 同名型をすべて確認する

```bash
qmlprop --all-matches Label
qmlprop --all-matches --debug Label
```

---

# 12. トラブルシューティング

## 12.1 `could not be instantiated` と出る

確認順は次の通りです。

1. `--debug` を付ける
2. `Reason:` と `Details:` を確認する
3. 型が required property を要求していないか確認する
4. 値型/抽象型/attached-only ではないか確認する
5. 必要な QML モジュールがインストールされているか確認する
6. 依存モジュール不足ではないか確認する

## 12.2 `No import profile is available for module: ...` と出る

意味は「その候補モジュールは見つかったが、現状の `qmlprop` はそのモジュールを試す import プロファイルを持っていない」です。

対処は次のいずれかです。

- 既存の別候補で目的の型が見られるか確認する
- `--debug` でどのプロファイルが使われたかを見る
- 必要なら `qmlprop` 側にそのモジュール用の import プロファイルを追加する

## 12.3 `Module:` が期待と違う

同名型が複数あると、単純な名前一致では誤解しやすいです。現在の `qmlprop` は、**成功した import プロファイルを優先して** `Module:` を決めるようにしています。

確認には次を使います。

```bash
qmlprop --debug <type>
```

## 12.4 値型を見たい

値型は通常の `QObject` ベースの QML オブジェクトとしては生成できません。通常の型レポート対象にはならず、`--list-types` の一覧にも出ません。一方で、生成失敗時の診断では値型として分類されることがあります。

---

# 13. 制限事項

- 環境にインストールされていない QML モジュールの型は生成できません。
- 一覧に見えるすべての型に対して import プロファイルが用意されているわけではありません。
- required property が必要な型は、そのままでは生成に失敗します。
- 値型/抽象型/attached-only 型は通常のオブジェクトとしては表示できません。
- 一部の型は import alias や追加の依存 import が必要です。
- `--all-matches` は候補列挙に有用ですが、内部モジュールや未対応モジュールも見えることがあります。
- `--all-matches` は import プロファイル未対応の候補は表示しますが、生成に失敗した候補を個別レポートとしては表示しません。

---

# 14. 終了ステータス

- 成功時: `0`
- 失敗時: `1`

スクリプトから使う場合は、この終了コードで成否判定できます。

---

# 15. まず覚えるコマンド

迷ったら、まずは次の5つで十分です。

```bash
qmlprop Rectangle
qmlprop --all Item
qmlprop --debug Label
qmlprop --show-attached Item
qmlprop --all-matches Label
```

これで大半の調査は始められます。
