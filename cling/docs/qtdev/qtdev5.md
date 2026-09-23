# Cling 実験コードと Qt Test 化の対応表

承知しました。
では、**重要な10案**を選び、各項目について

* **Cling で何を確認するか**
* **最小の Cling 実験コード例**
* **正式な Qt Test へどう落とすか**
* **何が分かるか**
* **注意点**

まで対応付けて整理します。

方針としては、特に実務で効きやすいものを優先して選びます。

---

# 1. `QStringView` / `QString` の境界確認

## Cling で確認すること

* 空文字列
* 一時オブジェクト由来の `QStringView`
* 部分文字列
* 日本語/絵文字
* 期待した比較結果になるか

## Cling 実験コード例

```cpp
#include <QString>
#include <QStringView>
#include <QDebug>

void inspectStringViews()
{
    QString empty;
    QString text = u"こんにちは世界"_qs;

    QStringView emptyView{empty};
    QStringView middleView{text.data() + 2, 3};

    qDebug() << "empty.isEmpty()" << empty.isEmpty();
    qDebug() << "emptyView.isEmpty()" << emptyView.isEmpty();
    qDebug() << "middleView" << middleView;
    qDebug() << "text.startsWith(こんにちは)" << text.startsWith(u"こんにちは");
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QString>
#include <QStringView>

class StringViewTest : public QObject
{
    Q_OBJECT

private slots:
    void emptyView();
    void substringView();
};

void StringViewTest::emptyView()
{
    QString text;
    QStringView view{text};

    QVERIFY(text.isEmpty());
    QVERIFY(view.isEmpty());
}

void StringViewTest::substringView()
{
    QString text = u"こんにちは世界"_qs;
    QStringView view{text.data() + 2, 3};

    QCOMPARE(view.toString(), u"にちは"_qs);
}
```

## 何が分かるか

* 何を期待値として固定すべきか
* `QStringView` の切り出し結果
* view の寿命が絡む危険箇所の候補

## 注意点

`QStringView` は所有しないので、**一時 `QString` から作った view を保持し続ける検証**は危険です。
Cling では動いて見えても、正式テストでは寿命を明確に分けて書くべきです。

---

# 2. `QVariant` / `QMetaType` の登録確認

## Cling で確認すること

* 独自型が `QVariant` に入るか
* `value<T>()` で取り出せるか
* `metaType()` の結果
* `canConvert()` の成否

## Cling 実験コード例

```cpp
#include <QVariant>
#include <QMetaType>
#include <QDebug>

struct SampleValue
{
    int number{};
};
Q_DECLARE_METATYPE(SampleValue)

void inspectVariantMetaType()
{
    qRegisterMetaType<SampleValue>("SampleValue");

    SampleValue value{42};
    QVariant variant = QVariant::fromValue(value);

    qDebug() << "typeName:" << variant.typeName();
    qDebug() << "metaType name:" << variant.metaType().name();
    qDebug() << "can convert to int:" << variant.canConvert<int>();

    SampleValue extracted = variant.value<SampleValue>();
    qDebug() << "extracted.number:" << extracted.number;
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QVariant>
#include <QMetaType>

struct SampleValue
{
    int number{};
    bool operator==(const SampleValue& other) const = default;
};
Q_DECLARE_METATYPE(SampleValue)

class VariantMetaTypeTest : public QObject
{
    Q_OBJECT

private slots:
    void roundTrip();
};

void VariantMetaTypeTest::roundTrip()
{
    qRegisterMetaType<SampleValue>("SampleValue");

    SampleValue value{42};
    QVariant variant = QVariant::fromValue(value);

    QVERIFY(variant.isValid());
    QCOMPARE(QString::fromLatin1(variant.metaType().name()), QStringLiteral("SampleValue"));
    QCOMPARE(variant.value<SampleValue>(), value);
}
```

## 何が分かるか

* 型登録の不足
* role データへ載せたときの型妥当性
* 比較可能にする必要があるか

## 注意点

正式テストでは、必要なら `operator==` を定義して `QCOMPARE` 可能にしておく方が扱いやすいです。

---

# 3. signal 発火回数の確認

## Cling で確認すること

* 値変更時に何回 signal が飛ぶか
* 同じ値の再設定で飛ぶか
* API 経由とユーザー操作経由で違いがあるか

## Cling 実験コード例

```cpp
#include <QLineEdit>
#include <QDebug>

void inspectSignalCount()
{
    auto* lineEdit = new QLineEdit;
    QObject::connect(lineEdit, &QLineEdit::textChanged,
                     [](const QString& text) {
                         qDebug() << "textChanged:" << text;
                     });

    lineEdit->setText("A");
    lineEdit->setText("A");
    lineEdit->setText("B");

    lineEdit->show();
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QLineEdit>
#include <QSignalSpy>

class SignalCountTest : public QObject
{
    Q_OBJECT

private slots:
    void textChangedBySetter();
};

void SignalCountTest::textChangedBySetter()
{
    QLineEdit lineEdit;
    QSignalSpy spy(&lineEdit, &QLineEdit::textChanged);

    lineEdit.setText("A");
    lineEdit.setText("A");
    lineEdit.setText("B");

    QCOMPARE(spy.count(), 2);
}
```

## 何が分かるか

* signal 回数の期待値
* 同値代入の扱い
* `QSignalSpy` のアサーション根拠

## 注意点

ここは**実際の挙動を見てから**書くのが重要です。
思い込みで `spy.count()` を決めると外しやすいです。

---

# 4. signal 発火順の確認

## Cling で確認すること

* 複数 signal の順序
* property 更新 signal と派生 signal の前後
* 独自オブジェクトの状態遷移順

## Cling 実験コード例

```cpp
#include <QObject>
#include <QDebug>

class StateEmitter : public QObject
{
    Q_OBJECT

public:
    void setValue(int newValue)
    {
        if (value == newValue) {
            return;
        }
        value = newValue;
        emit valueChanged(value);
        emit stateUpdated();
    }

signals:
    void valueChanged(int value);
    void stateUpdated();

private:
    int value = 0;
};

void inspectSignalOrder()
{
    auto* emitter = new StateEmitter;

    QObject::connect(emitter, &StateEmitter::valueChanged,
                     [](int value) { qDebug() << "valueChanged" << value; });
    QObject::connect(emitter, &StateEmitter::stateUpdated,
                     []() { qDebug() << "stateUpdated"; });

    emitter->setValue(10);
}

#include "state_emitter.moc"
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QObject>

class StateEmitter : public QObject
{
    Q_OBJECT

public:
    void setValue(int newValue)
    {
        if (value == newValue) {
            return;
        }
        value = newValue;
        emit valueChanged(value);
        emit stateUpdated();
    }

signals:
    void valueChanged(int value);
    void stateUpdated();

private:
    int value = 0;
};

class SignalOrderTest : public QObject
{
    Q_OBJECT

private slots:
    void order();
};

void SignalOrderTest::order()
{
    StateEmitter emitter;
    QStringList eventLog;

    connect(&emitter, &StateEmitter::valueChanged, this,
            [&eventLog](int value) { eventLog << QString("valueChanged:%1").arg(value); });
    connect(&emitter, &StateEmitter::stateUpdated, this,
            [&eventLog]() { eventLog << "stateUpdated"; });

    emitter.setValue(10);

    QCOMPARE(eventLog, QStringList({ "valueChanged:10", "stateUpdated" }));
}
```

## 何が分かるか

* 正式テストで順序まで固定すべきか
* signal の責務分離が妥当か
* API 設計の改善余地

## 注意点

順序を固定するテストは強い制約になるため、**本当に契約として保証したい場合だけ**固定する方がよいです。

---

# 5. `deleteLater()` のタイミング確認

## Cling で確認すること

* `deleteLater()` 後すぐには消えないこと
* イベントループを1回回すとどうなるか
* `QPointer` がいつ `nullptr` になるか

## Cling 実験コード例

```cpp
#include <QObject>
#include <QPointer>
#include <QCoreApplication>
#include <QDebug>

void inspectDeleteLater()
{
    auto* object = new QObject;
    QPointer<QObject> guardedPointer = object;

    QObject::connect(object, &QObject::destroyed, []() {
        qDebug() << "destroyed";
    });

    object->deleteLater();
    qDebug() << "after deleteLater:" << (guardedPointer != nullptr);

    QCoreApplication::processEvents();
    qDebug() << "after processEvents:" << (guardedPointer != nullptr);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    qDebug() << "after sendPostedEvents:" << (guardedPointer != nullptr);
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QObject>
#include <QPointer>
#include <QCoreApplication>

class DeleteLaterTest : public QObject
{
    Q_OBJECT

private slots:
    void deferredDelete();
};

void DeleteLaterTest::deferredDelete()
{
    auto* object = new QObject;
    QPointer<QObject> guardedPointer = object;

    object->deleteLater();
    QVERIFY(guardedPointer != nullptr);

    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    QVERIFY(guardedPointer == nullptr);
}
```

## 何が分かるか

* `deleteLater()` の本当の意味
* ライフタイム不具合の再現条件
* `QPointer` を使うべき箇所

## 注意点

`processEvents()` だけで期待通り消えるとは限らない場面があります。
**`DeferredDelete` を明示的に流すか** を設計として決めた方がよいです。

---

# 6. `QTimer::singleShot()` / 非同期順序の確認

## Cling で確認すること

* 0ms の `singleShot` がどの順で動くか
* 複数予約時の順序
* 直後実行ではなくイベントループ後であること

## Cling 実験コード例

```cpp
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

void inspectSingleShotOrder()
{
    QStringList eventLog;

    QTimer::singleShot(0, [&]() {
        eventLog << "first";
        qDebug() << eventLog;
    });

    QTimer::singleShot(0, [&]() {
        eventLog << "second";
        qDebug() << eventLog;
    });

    qDebug() << "before processEvents" << eventLog;
    QCoreApplication::processEvents();
    qDebug() << "after processEvents" << eventLog;
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QTimer>

class TimerOrderTest : public QObject
{
    Q_OBJECT

private slots:
    void zeroTimeoutOrder();
};

void TimerOrderTest::zeroTimeoutOrder()
{
    QStringList eventLog;

    QTimer::singleShot(0, [&]() { eventLog << "first"; });
    QTimer::singleShot(0, [&]() { eventLog << "second"; });

    QTRY_COMPARE(eventLog.size(), 2);
    QCOMPARE(eventLog, QStringList({ "first", "second" }));
}
```

## 何が分かるか

* 非同期コードの最小再現条件
* `QTRY_COMPARE` を使うべき場面
* 即時処理と遅延処理の切り分け

## 注意点

順序依存は環境差に弱い場合があります。
順序を保証したいのか、単に「両方走る」ことだけで十分かを分けて考えるべきです。

---

# 7. ウィジェット初期状態の確認

## Cling で確認すること

* コンストラクター直後の状態
* テキスト
* enabled/visible
* フォーカスポリシー
* `sizeHint()`

## Cling 実験コード例

```cpp
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>

class LoginWidget : public QWidget
{
public:
    LoginWidget()
    {
        auto* layout = new QVBoxLayout(this);
        userNameEdit = new QLineEdit;
        loginButton = new QPushButton("Login");

        loginButton->setEnabled(false);

        layout->addWidget(userNameEdit);
        layout->addWidget(loginButton);
    }

    QLineEdit* userNameEdit = nullptr;
    QPushButton* loginButton = nullptr;
};

void inspectInitialWidgetState()
{
    auto* widget = new LoginWidget;
    qDebug() << "button enabled:" << widget->loginButton->isEnabled();
    qDebug() << "line edit text:" << widget->userNameEdit->text();
    qDebug() << "sizeHint:" << widget->sizeHint();
    widget->show();
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

class LoginWidget : public QWidget
{
public:
    LoginWidget()
    {
        auto* layout = new QVBoxLayout(this);
        userNameEdit = new QLineEdit;
        loginButton = new QPushButton("Login");
        loginButton->setEnabled(false);
        layout->addWidget(userNameEdit);
        layout->addWidget(loginButton);
    }

    QLineEdit* userNameEdit = nullptr;
    QPushButton* loginButton = nullptr;
};

class WidgetInitialStateTest : public QObject
{
    Q_OBJECT

private slots:
    void initialState();
};

void WidgetInitialStateTest::initialState()
{
    LoginWidget widget;

    QVERIFY(widget.userNameEdit != nullptr);
    QVERIFY(widget.loginButton != nullptr);
    QVERIFY(widget.userNameEdit->text().isEmpty());
    QVERIFY(!widget.loginButton->isEnabled());
}
```

## 何が分かるか

* 初期状態として固定すべき項目
* テスト対象にすべき public 状態
* コンストラクターの責務

## 注意点

表示依存の値は、必要なら `show()` と `QTest::qWaitForWindowExposed()` のような段階を分けてテストした方が安全です。

---

# 8. `QAbstractListModel` の基本契約確認

## Cling で確認すること

* `rowCount()`
* `data()`
* `roleNames()`
* 範囲外 index の扱い
* 空状態

## Cling 実験コード例

```cpp
#include <QAbstractListModel>
#include <QDebug>

class NameListModel : public QAbstractListModel
{
public:
    enum Role {
        NameRole = Qt::UserRole + 1
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            return 0;
        }
        return names.size();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= names.size()) {
            return {};
        }
        if (role == NameRole) {
            return names.at(index.row());
        }
        return {};
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return { { NameRole, "name" } };
    }

    QStringList names { "Alice", "Bob" };
};

void inspectListModel()
{
    NameListModel model;

    qDebug() << "rowCount" << model.rowCount();
    qDebug() << "roleNames" << model.roleNames();
    qDebug() << "index 0 name" << model.data(model.index(0, 0), NameListModel::NameRole);
    qDebug() << "invalid index" << model.data(model.index(99, 0), NameListModel::NameRole);
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QAbstractListModel>

class NameListModel : public QAbstractListModel
{
public:
    enum Role {
        NameRole = Qt::UserRole + 1
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : names.size();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= names.size()) {
            return {};
        }
        return role == NameRole ? QVariant(names.at(index.row())) : QVariant{};
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return { { NameRole, "name" } };
    }

    QStringList names { "Alice", "Bob" };
};

class ListModelContractTest : public QObject
{
    Q_OBJECT

private slots:
    void basicContract();
};

void ListModelContractTest::basicContract()
{
    NameListModel model;

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.roleNames().value(NameListModel::NameRole), QByteArray("name"));
    QCOMPARE(model.data(model.index(0, 0), NameListModel::NameRole).toString(), QString("Alice"));
    QVERIFY(!model.data(model.index(99, 0), NameListModel::NameRole).isValid());
}
```

## 何が分かるか

* model の基本 contract が成立しているか
* QML へ出す前の整合性
* role 名の設計ミス

## 注意点

Model/View は「見た目が動く」だけでは不十分で、**範囲外 index や親 index の扱い**も明示的に見るべきです。

---

# 9. `beginInsertRows()` / `endInsertRows()` の整合確認

## Cling で確認すること

* 行追加前後の件数変化
* 通知順
* 挿入範囲が正しいか

## Cling 実験コード例

```cpp
#include <QAbstractListModel>
#include <QDebug>

class EditableNameListModel : public QAbstractListModel
{
public:
    enum Role {
        NameRole = Qt::UserRole + 1
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : names.size();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() >= names.size()) {
            return {};
        }
        return role == NameRole ? QVariant(names.at(index.row())) : QVariant{};
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return { { NameRole, "name" } };
    }

    void appendName(const QString& name)
    {
        const int insertRow = names.size();
        beginInsertRows(QModelIndex(), insertRow, insertRow);
        names.append(name);
        endInsertRows();
    }

    QStringList names;
};

void inspectInsertRows()
{
    EditableNameListModel model;

    QObject::connect(&model, &QAbstractItemModel::rowsInserted,
                     [](const QModelIndex&, int first, int last) {
                         qDebug() << "rowsInserted" << first << last;
                     });

    qDebug() << "before" << model.rowCount();
    model.appendName("Alice");
    qDebug() << "after" << model.rowCount();
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QAbstractListModel>
#include <QSignalSpy>

class EditableNameListModel : public QAbstractListModel
{
public:
    enum Role {
        NameRole = Qt::UserRole + 1
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : names.size();
    }

    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || index.row() >= names.size()) {
            return {};
        }
        return role == NameRole ? QVariant(names.at(index.row())) : QVariant{};
    }

    QHash<int, QByteArray> roleNames() const override
    {
        return { { NameRole, "name" } };
    }

    void appendName(const QString& name)
    {
        const int insertRow = names.size();
        beginInsertRows(QModelIndex(), insertRow, insertRow);
        names.append(name);
        endInsertRows();
    }

    QStringList names;
};

class InsertRowsTest : public QObject
{
    Q_OBJECT

private slots:
    void appendOneRow();
};

void InsertRowsTest::appendOneRow()
{
    EditableNameListModel model;
    QSignalSpy spy(&model, &QAbstractItemModel::rowsInserted);

    model.appendName("Alice");

    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.data(model.index(0, 0), EditableNameListModel::NameRole).toString(),
             QString("Alice"));
    QCOMPARE(spy.count(), 1);
}
```

## 何が分かるか

* 通知漏れの有無
* model 更新 contract の妥当性
* view/QML が更新されない原因の切り分け

## 注意点

件数だけでなく、**signal が飛んだか**も見ないと contract 不備を見逃します。

---

# 10. QML property / binding 更新確認

## Cling で確認すること

* QML 側の初期値
* C++ からの `setProperty()` 反映
* binding が維持されるか
* model 更新後に view が変わるか

## Cling 実験コード例

```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>

void inspectQmlPropertyBinding()
{
    QQmlEngine engine;

    const char* qmlText = R"(
        import QtQuick

        Rectangle {
            width: 200
            height: 100

            property int baseWidth: 200

            Rectangle {
                objectName: "childRect"
                width: parent.baseWidth / 2
                height: 20
            }
        }
    )";

    QQmlComponent component(&engine);
    component.setData(qmlText, QUrl());

    QObject* rootObject = component.create();
    auto* childObject = rootObject->findChild<QObject*>("childRect");

    qDebug() << "initial child width:" << childObject->property("width");

    rootObject->setProperty("baseWidth", 300);
    qDebug() << "after baseWidth change:" << childObject->property("width");

    childObject->setProperty("width", 123);
    rootObject->setProperty("baseWidth", 400);
    qDebug() << "after imperative override and baseWidth change:" << childObject->property("width");

    delete rootObject;
}
```

## Qt Test への落とし込み例

```cpp
#include <QtTest>
#include <QQmlEngine>
#include <QQmlComponent>

class QmlBindingTest : public QObject
{
    Q_OBJECT

private slots:
    void propertyBinding();
};

void QmlBindingTest::propertyBinding()
{
    QQmlEngine engine;

    const char* qmlText = R"(
        import QtQuick

        Rectangle {
            width: 200
            height: 100

            property int baseWidth: 200

            Rectangle {
                objectName: "childRect"
                width: parent.baseWidth / 2
                height: 20
            }
        }
    )";

    QQmlComponent component(&engine);
    component.setData(qmlText, QUrl());
    std::unique_ptr<QObject> rootObject(component.create());

    QVERIFY(rootObject != nullptr);

    QObject* childObject = rootObject->findChild<QObject*>("childRect");
    QVERIFY(childObject != nullptr);

    QCOMPARE(childObject->property("width").toInt(), 100);

    rootObject->setProperty("baseWidth", 300);
    QCOMPARE(childObject->property("width").toInt(), 150);

    childObject->setProperty("width", 123);
    rootObject->setProperty("baseWidth", 400);
    QCOMPARE(childObject->property("width").toInt(), 123);
}
```

## 何が分かるか

* binding 成立/破壊条件
* QML テストの比較対象
* C++/QML 境界の責務

## 注意点

QML の binding は imperative assignment で壊れることがあるので、**意図した仕様かどうか**を先に Cling で確認してから正式テスト化するのが有効です。

---

# まとめ: Cling と Qt Test の役割分担

この10項目を通すと、役割分担はかなり明確です。

## Cling が向いていること

* 仕様観察
* 境界条件探索
* signal/イベント順の把握
* 最小再現作成
* QML binding の切り分け

## Qt Test が向いていること

* 期待値の固定
* 回帰防止
* CI 実行
* 長期保守
* 機械的な再検証

つまり、実務では

1. **Cling で挙動を確かめる**
2. **期待値を確定する**
3. **Qt Test に落とす**

の流れが最も自然です。

---

# 優先して整備するとよい10テーマ

特に優先度が高いのは次です。

1. `QStringView`
2. `QVariant/QMetaType`
3. signal 回数
4. signal 順序
5. `deleteLater`
6. `QTimer::singleShot`
7. ウィジェット初期状態
8. `QAbstractListModel` 基本契約
9. `beginInsertRows/endInsertRows`
10. QML property/binding

この10本をそろえると、かなり広い範囲をカバーできます。

次に進めるなら、
**この10本について、Cling でそのまま実行しやすいように共通ヘルパーを付けたサンプル集構成案** を作るのが実用的です。

