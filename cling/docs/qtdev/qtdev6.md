# Cling でそのまま実行しやすい共通ヘルパーを付けたサンプル集構成案

承知しました。
では、**この10本を Cling でそのまま試しやすくするための共通ヘルパー付きサンプル集構成案** を、実運用しやすい形でまとめます。

目的は次の3つです。

1. **Cling で即実験できること**
2. **後で Qt Test へ移しやすいこと**
3. **Widgets/QML/非 GUI を横断して再利用できること**

---

# 1. 全体方針

このサンプル集は、単発の断片コードではなく、**共通ヘルパーを読み込んでから個別テーマを試す構成** にすると使いやすいです。

イメージとしては次の3層です。

## 1. 共通基盤

* Qt 初期化
* イベント処理補助
* ログ補助
* signal 記録補助
* QObject/Model/QML の観察補助

## 2. 個別テーマ

* `QStringView`
* `QVariant/QMetaType`
* signal 回数
* signal 順序
* `deleteLater`
* `QTimer`
* ウィジェット初期状態
* `QAbstractListModel`
* `beginInsertRows()/endInsertRows()`
* QML property/binding

## 3. 正式テスト化テンプレート

* `QVERIFY`
* `QCOMPARE`
* `QSignalSpy`
* `QTRY_COMPARE`

へ変換しやすい形の対応例

---

# 2. ディレクトリー構成案

例えば次のように分けると管理しやすいです。

```text
cling-qt-test-lab/
├─ bootstrap/
│  ├─ qt_bootstrap.cpp
│  ├─ qt_helpers.cpp
│  ├─ signal_helpers.cpp
│  ├─ object_helpers.cpp
│  ├─ model_helpers.cpp
│  └─ qml_helpers.cpp
│
├─ experiments/
│  ├─ 01_qstringview_cases.cpp
│  ├─ 02_qvariant_metatype.cpp
│  ├─ 03_signal_count.cpp
│  ├─ 04_signal_order.cpp
│  ├─ 05_delete_later.cpp
│  ├─ 06_timer_order.cpp
│  ├─ 07_widget_initial_state.cpp
│  ├─ 08_list_model_contract.cpp
│  ├─ 09_insert_rows_contract.cpp
│  └─ 10_qml_binding.cpp
│
├─ test_templates/
│  ├─ template_non_gui_qttest.cpp
│  ├─ template_signal_qttest.cpp
│  ├─ template_widget_qttest.cpp
│  ├─ template_model_qttest.cpp
│  └─ template_qml_qttest.cpp
│
└─ README.md
```

この形だと、Cling 用の探索コードと Qt Test への移行先が分かれ、役割が明確です。

---

# 3. 共通基盤で用意したいもの

ここが重要です。
個別サンプルを楽にするには、最初に**共通で読み込む補助関数**を整えるのが効果的です。

---

## 3.1 Qt 初期化ヘルパー

Cling で毎回 `QApplication` や `QGuiApplication` の準備を書くのは面倒です。
そのため、まず **Qt を起動済み状態にする補助** を置きます。

### 役割

* `QCoreApplication`/`QApplication` の存在確認
* 必要なら生成
* 引数の簡易準備
* 再利用

### あるとよい関数名

* `ensureCoreApplication()`
* `ensureGuiApplication()`
* `ensureWidgetsApplication()`

### 効果

各サンプルが短くなります。

---

## 3.2 イベント処理補助

単体テスト支援ではイベント処理が多いです。

### あるとよい関数

* `processEventsOnce()`
* `processDeferredDeleteEvents()`
* `drainEventQueue()`
* `waitMilliseconds(int milliseconds)`

### 用途

* `deleteLater()` の確認
* `QTimer::singleShot()` の確認
* signal 発火待ち
* QML 更新待ち

---

## 3.3 ログ整形補助

`qDebug()` だけでも十分ですが、テーマごとに出力形式をそろえると見やすいです。

### あるとよい関数

* `printSectionTitle(const QString& title)`
* `printCheckResult(const QString& label, const QVariant& value)`
* `printBooleanResult(const QString& label, bool value)`

### 効果

探索結果を後で正式テストへ写しやすくなります。

---

## 3.4 signal 記録ヘルパー

これはかなり重要です。
signal の発火回数や順序をその場で調べるための軽量ロガーを置きます。

### あるとよいもの

* 文字列ログをためる `SignalEventLog`
* ラムダ接続を簡単に書ける補助
* signal 名と引数を記録する補助

### あるとよい関数名

* `makeSignalEventLog()`
* `appendEvent(QString eventText)`
* `printEventLog(const QStringList& eventLog)`

### 効果

`QSignalSpy` を書く前の探索が楽になります。

---

## 3.5 QObject 観察ヘルパー

QObject 状態を見やすくする補助です。

### 見たい項目

* `objectName`
* クラス名
* 親
* 子数
* thread affinity
* dynamic property
* 主要 property

### あるとよい関数

* `dumpObjectTree(QObject* rootObject)`
* `dumpObjectSummary(QObject* object)`
* `dumpDynamicProperties(QObject* object)`

### 用途

* ownership 調査
* `deleteLater()` 調査
* Widgets/QML の両方で利用

---

## 3.6 Model 観察ヘルパー

Model/View 系は、補助関数があると一気に使いやすくなります。

### 見たい項目

* `rowCount()`
* `columnCount()`
* role 名
* 各 index の `data()`
* header 値

### あるとよい関数

* `dumpModelRoles(const QAbstractItemModel* model)`
* `dumpListModelData(const QAbstractItemModel* model, int role)`
* `dumpTableModelData(const QAbstractItemModel* model)`

### 用途

* `QAbstractListModel` 契約確認
* `beginInsertRows()/endInsertRows()` 確認
* QML 連携前の確認

---

## 3.7 QML 観察ヘルパー

QML は C++ 側から見る補助をそろえると便利です。

### 見たい項目

* ルート作成成否
* child 検索
* property 値
* item 階層

### あるとよい関数

* `createQmlObject(QQmlEngine* engine, const QByteArray& qmlSourceText)`
* `findRequiredChild(QObject* rootObject, const QString& objectName)`
* `printProperty(QObject* object, const char* propertyName)`
* `dumpQmlObjectTree(QObject* rootObject)`

### 用途

* QML binding
* property 更新
* model 反映確認

---

# 4. サンプル10本の構成テンプレート

各ファイルの形をそろえると、読みやすくなります。

各実験ファイルは次のような構造にすると扱いやすいです。

```cpp
void runExperiment()
{
    printSectionTitle("...");
    ensureWidgetsApplication();   // または ensureGuiApplication()
    ...
    printCheckResult("...", ...);
}
```

さらに、各サンプルに次の4ブロックを入れると後で再利用しやすいです。

1. **目的**
2. **入力ケース**
3. **観察ポイント**
4. **Qt Test 化するときの論点**

コメントとして残しておくと便利です。

---

# 5. 各サンプルに付けたい補助

10本それぞれについて、あると便利な共通補助を対応付けます。

---

## 5.1 `01_qstringview_cases.cpp`

### 使う補助

* `printSectionTitle`
* `printCheckResult`

### 特化してあるとよい補助

* `printStringViewInfo(QStringView textView)`

### 出したい情報

* 長さ
* 空かどうか
* `toString()`
* 元文字列との関係

---

## 5.2 `02_qvariant_metatype.cpp`

### 使う補助

* `printCheckResult`

### 特化してあるとよい補助

* `printVariantMetaInfo(const QVariant& variant)`

### 出したい情報

* `isValid()`
* `typeName()`
* `metaType().name()`
* `canConvert<T>()`

---

## 5.3 `03_signal_count.cpp`

### 使う補助

* `SignalEventLog`
* `printEventLog`

### 特化してあるとよい補助

* `connectTextChangedLogger(QLineEdit* lineEdit, QStringList* eventLog)`

### 出したい情報

* 発火回数
* 引数の内容

---

## 5.4 `04_signal_order.cpp`

### 使う補助

* `SignalEventLog`
* `printEventLog`

### 特化してあるとよい補助

* `appendOrderedEvent(QStringList* eventLog, const QString& eventText)`

### 出したい情報

* 発火順
* event log 全体

---

## 5.5 `05_delete_later.cpp`

### 使う補助

* `processDeferredDeleteEvents`
* `dumpObjectSummary`

### 特化してあるとよい補助

* `printPointerAliveState(const QString& label, const QPointer<QObject>& guardedPointer)`

### 出したい情報

* `deleteLater()` 直後
* `processEvents()` 後
* `DeferredDelete` 後

---

## 5.6 `06_timer_order.cpp`

### 使う補助

* `waitMilliseconds`
* `drainEventQueue`
* `printEventLog`

### 特化してあるとよい補助

* `runEventLoopUntil(const std::function<bool()>& predicate)`

### 出したい情報

* 処理前ログ
* 処理後ログ
* 実行順

---

## 5.7 `07_widget_initial_state.cpp`

### 使う補助

* `ensureWidgetsApplication`
* `dumpObjectSummary`

### 特化してあるとよい補助

* `printWidgetGeometry(QWidget* widget)`
* `printWidgetState(QWidget* widget)`

### 出したい情報

* `isEnabled()`
* `isVisible()`
* `sizeHint()`
* 初期 text

---

## 5.8 `08_list_model_contract.cpp`

### 使う補助

* `dumpModelRoles`
* `dumpListModelData`

### 特化してあるとよい補助

* `printModelIndexState(const QModelIndex& modelIndex)`

### 出したい情報

* 行数
* role 名
* 有効/無効 index
* `data()` 結果

---

## 5.9 `09_insert_rows_contract.cpp`

### 使う補助

* `SignalEventLog`
* `dumpListModelData`

### 特化してあるとよい補助

* `connectRowsInsertedLogger(QAbstractItemModel* model, QStringList* eventLog)`

### 出したい情報

* 挿入前件数
* 挿入後件数
* `rowsInserted` の引数
* 実データ

---

## 5.10 `10_qml_binding.cpp`

### 使う補助

* `ensureGuiApplication`
* `createQmlObject`
* `findRequiredChild`
* `printProperty`

### 特化してあるとよい補助

* `printQmlBindingCheck(QObject* object, const char* propertyName, const QString& label)`

### 出したい情報

* 初期 property 値
* C++ から変更後の値
* imperative assignment 後の値

---

# 6. README に書くべき内容

サンプル集として使いやすくするには、README が重要です。

最低限、次を入れるとよいです。

## 6.1 目的

* Cling は正式テストの置き換えではない
* 仕様探索/最小再現/テスト原案作成に使う

## 6.2 使い方

例えば、

```cpp
.L bootstrap/qt_bootstrap.cpp
.L bootstrap/qt_helpers.cpp
.L bootstrap/signal_helpers.cpp
.L experiments/03_signal_count.cpp
runExperiment()
```

のような流れです。

## 6.3 各サンプルの位置付け

* 何を観察するサンプルか
* Qt Test では何に変換するか

## 6.4 注意点

* Cling 上の観察結果をそのまま CI 契約にしない
* signal 順序固定は慎重に
* QML binding は imperative assignment に注意
* ライフタイム系はイベント処理の段階を明確にする

---

# 7. Qt Test 化テンプレートとの対応

サンプル集の価値を上げるには、「探索から正式テストへ」の橋渡しを明確にすることです。

そのため、各サンプルに**対応する Qt Test テンプレート**を用意しておくとよいです。

---

## 7.1 非 GUI 用テンプレート

向いているテーマ:

* `QStringView`
* `QVariant/QMetaType`

主なアサーション:

* `QVERIFY`
* `QCOMPARE`

---

## 7.2 signal 用テンプレート

向いているテーマ:

* signal 回数
* signal 順序
* `QTimer`

主なアサーション:

* `QSignalSpy`
* `QCOMPARE`
* `QTRY_COMPARE`

---

## 7.3 Widget 用テンプレート

向いているテーマ:

* 初期状態
* setter 後状態

主なアサーション:

* `QVERIFY`
* `QCOMPARE`
* 必要に応じて `QTest::qWaitForWindowExposed`

---

## 7.4 Model 用テンプレート

向いているテーマ:

* 基本契約
* 挿入通知

主なアサーション:

* `QCOMPARE(model.rowCount(), ...)`
* `QSignalSpy`
* role 名比較
* `data()` 比較

---

## 7.5 QML 用テンプレート

向いているテーマ:

* property
* binding
* model 反映

主なアサーション:

* `QQmlEngine`
* `QQmlComponent`
* `QObject::property`
* `QCOMPARE`

---

# 8. 最初に作るべき最小セット

全部を最初からそろえなくてもよいです。
まずは次の最小セットで十分実用的です。

## bootstrap

* `qt_bootstrap.cpp`
* `qt_helpers.cpp`
* `signal_helpers.cpp`
* `model_helpers.cpp`
* `qml_helpers.cpp`

## experiments

* `03_signal_count.cpp`
* `05_delete_later.cpp`
* `06_timer_order.cpp`
* `08_list_model_contract.cpp`
* `10_qml_binding.cpp`

この5本は、Qt の単体テスト設計で特に詰まりやすい部分をかなりカバーします。

---

# 9. 優先順位

実務で整備順を付けるなら次です。

## 第1段階

* Qt 初期化
* イベント処理補助
* signal ログ
* `deleteLater`
* `QTimer`

## 第2段階

* Model ダンプ
* `QAbstractListModel`
* `beginInsertRows()/endInsertRows()`

## 第3段階

* QML 生成補助
* QML property/binding 確認

## 第4段階

* Widgets 初期状態
* `QStringView`
* `QVariant/QMetaType`

---

# 10. 結論

このサンプル集は、単なる実験集ではなく、次の流れを支える基盤になります。

1. **Cling で挙動を探索する**
2. **観察結果を整理する**
3. **正式な Qt Test に落とす**
4. **バグ再現や教材にも流用する**

特に重要なのは、**共通ヘルパーを先に整備して、個別サンプルを短く保つこと**です。
そうすると、サンプル集が育ちやすくなります。

次に進めるなら、実用性が高いのは
**この構成案をもとにした `README.md` の草案** か、
**最初に作るべき bootstrap 5ファイルの雛形案** です。

