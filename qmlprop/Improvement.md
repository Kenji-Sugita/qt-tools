## qmlprop 改善メモ

この文書は、現行実装を前提にした改善候補の整理です。
すでに入っている機能と、これから検討すべき機能を分けて記録します。

---

## 1. すでに実装済みの項目

次の項目は、改善案としては有効でしたが、現時点ではすでに実装済みです。

1. 継承メンバーを含める/含めない切り替え

- 既定ではその型自身のメンバー中心です。
- `--all` で継承メンバーも表示できます。

2. シグナル/スロット/通常メソッドの区別

- `Methods:` では `method` / `signal` / `slot` を表示しています。

3. 生成失敗理由の分類表示

- `could not be instantiated` に加えて、`Reason:` / `Details:` を表示します。
- 代表的には `required property not set`、`value type`、`abstract type`、`attached-only type`、`module import missing`、`dependent type missing`、`uncreatable type` を分類します。

4. enum プロパティー値の名前表示

- enum 型のプロパティー値は、数値だけでなくキー名も使って表示します。

5. 利用可能なモジュール一覧

- `--list-modules` で、現在の実行環境で見えている QML モジュール URI を一覧表示できます。

6. モジュール全体の索引生成

- `--module-index <module>` で、指定モジュール内の型を 1 行 1 型の表で表示できます。
- 現在は次の列を表示します。
  - 型名
  - 基底クラス名
  - 型フラグ
  - その型自身のプロパティー数
  - その型自身のメソッド数
  - その型自身の列挙子数
  - デフォルトプロパティー名
  - `Status`

補足:

- これは QML 型の索引です。C++ API 中心のモジュールでは件数がかなり少ないことがあります。
- `Status: no profile` は、型カタログ上は見えているが `qmlprop` 側に対応 import プロファイルが無いことを意味します。
- 画面必須型を多く含むモジュールでは、実行環境によって索引途中で失敗することがあります。

---

## 2. 未着手の改善候補

以下は、現時点では未実装で、追加価値がある候補です。

### 2.1 プロパティー関連

1. notify signal の表示

たとえば次のように表示できると便利です。

```text
text: QString = "Hello" notify=textChanged
```

用途:

- バインディング更新の有無をすぐ確認できる
- 監視すべき signal を把握しやすい

2. required プロパティー一覧だけを先に出す機能

たとえば次のような出力です。

```text
Required properties:
    model: QAbstractItemModel*
    delegate: QQmlComponent*
```

用途:

- その型を最小構成で生成する準備がしやすい
- required property 失敗を減らせる

### 2.2 使い方支援

3. 型ごとの import 例表示

たとえば `Page` に対して:

```text
Import:
    import QtQuick.Controls
```

用途:

- 調べた型をそのまま QML に書き戻しやすい
- `Module:` より直接的にコピペしやすい

4. 最小インスタンス化用の QML スニペット生成

たとえば:

```text
Example:
    import QtQuick
    Rectangle {
    }
```

required property がある型では、その雛形も含めたいです。

用途:

- ドキュメント確認から試作までの往復を減らせる

### 2.3 一覧・検索

5. `--list-types` のフィルター強化

候補:

- `--module QtQuick`
- `--name '*Layout*'`
- `--creatable-only`
- `--singleton-only`
- `--attached-provider-only`

用途:

- 型一覧の実用性がかなり上がる
- 外部の `grep` だけでは拾いにくい条件で絞り込める

6. 特定プロパティー名/シグナル名の検索

たとえば:

```bash
qmlprop --find-property contentItem
qmlprop --find-signal clicked
```

用途:

- 「どの型がその名前を持っているか」を逆引きできる

### 2.4 出力・連携

7. 出力形式の切り替え

候補:

- `text`
- `json`
- `yaml`

特に `json` は価値が高いです。

用途:

- 外部ツールとの連携
- 検索、差分比較、自動整形

8. 2 型の比較機能

たとえば:

```bash
qmlprop --diff Item Rectangle
```

見たい内容:

- `Rectangle` が `Item` に対して増やしているプロパティー
- 追加された enum
- 追加された signal

用途:

- 継承差分の確認
- API 学習

### 2.5 より大きい拡張

9. バージョン別の存在確認

たとえば:

- この型は Qt 6.11 で存在するか
- Qt 6.8 ではあるか
- どの import に属するか

用途:

- 複数 Qt 環境での移植確認

10. 実行時オブジェクトのダンプ

型ではなく、実際の QML オブジェクトツリーに対して:

- `objectName`
- クラス
- 親子関係
- 主要プロパティー値

を出す機能です。

用途:

- 型調査ツールからデバッグ補助ツールへの拡張

---

## 3. 優先順位の見直し

現時点で優先度が高いのは次の4つです。

1. `--list-types` のフィルター強化
2. `json` 出力
3. required プロパティー一覧
4. 型ごとの import 例表示

理由:

- `--list-types` のフィルター強化は、既存の型カタログ機能をほぼそのまま伸ばせるので費用対効果が高いです。
- `json` 出力は、後段の検索、比較、整形を外部に逃がせるので拡張性が高いです。
- required プロパティー一覧は、失敗診断の次の一手として自然です。
- import 例表示は、日常利用の摩擦をかなり減らせます。

次点候補は次の通りです。

1. notify signal 表示
2. 2 型比較
3. プロパティー名/シグナル名検索
4. 最小 QML スニペット生成

大きめの拡張として後回しでもよいのは次です。

1. バージョン別存在確認
2. 実行時オブジェクトダンプ

---

## 4. 次に着手するなら

次の 1 件を選ぶなら、`json` 出力を優先する価値が高いです。

理由:

- 既存のテキスト出力を壊さず追加しやすい
- 検索、差分、可視化を外部ツールに委ねやすい
- 将来の `--diff` や索引生成の基盤にもなりやすい

次点は `--list-types` のフィルター強化です。
