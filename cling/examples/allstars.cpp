// main.cpp (Qt 6 / Widgets)
// 目的: Squish for Qt の動作確認用に、よく使うウィジェットを広く配置するサンプル
#include <QApplication>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTableView>
#include <QTextEdit>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QTimer>

static QStandardItemModel* createTableModel(QObject* parent)
{
    auto* model = new QStandardItemModel(5, 3, parent);
    model->setHorizontalHeaderLabels({"Name", "Value", "Note"});
    for (int r = 0; r < model->rowCount(); ++r) {
        model->setItem(r, 0, new QStandardItem(QString("Item %1").arg(r)));
        model->setItem(r, 1, new QStandardItem(QString::number(r * 10)));
        model->setItem(r, 2, new QStandardItem("Editable"));
    }
    return model;
}

static QStandardItemModel* createTreeModel(QObject* parent)
{
    auto* model = new QStandardItemModel(parent);
    model->setHorizontalHeaderLabels({"Title", "Detail"});
    auto* root = model->invisibleRootItem();

    for (int i = 0; i < 3; ++i) {
        auto* parentItem = new QStandardItem(QString("Group %1").arg(i));
        auto* parentDetail = new QStandardItem("Parent node");
        QList<QStandardItem*> row{parentItem, parentDetail};
        root->appendRow(row);

        for (int j = 0; j < 4; ++j) {
            auto* childItem = new QStandardItem(QString("Child %1-%2").arg(i).arg(j));
            auto* childDetail = new QStandardItem("Child node");
            parentItem->appendRow({childItem, childDetail});
        }
    }
    return model;
}

class MainWindow : public QMainWindow
{
public:
    MainWindow()
    {
        setObjectName("mwMainWindow");
        setWindowTitle("Squish for Qt Widget Playground");
        resize(1100, 700);

        setupMenuToolBarStatus();
        setupDock();
        setupCentral();

        // ちょっとした動き(進捗バー更新)を入れて、同期/待機系の確認もしやすくします
        auto* timer = new QTimer(this);
        timer->setObjectName("tmProgressTimer");
        connect(timer, &QTimer::timeout, this, [this]() {
            int v = m_progressBar->value();
            v = (v + 3) % 101;
            m_progressBar->setValue(v);
            m_lcd->display(v);
        });
        timer->start(120);
    }

private:
    QProgressBar* m_progressBar = nullptr;
    QLCDNumber* m_lcd = nullptr;

    void setupMenuToolBarStatus()
    {
        auto* actAbout = new QAction("About", this);
        actAbout->setObjectName("actAbout");
        connect(actAbout, &QAction::triggered, this, [this]() {
            QMessageBox::information(this, "About", "Widget playground for Squish for Qt.");
        });

        auto* actOpenDialog = new QAction("Open Dialog", this);
        actOpenDialog->setObjectName("actOpenDialog");
        connect(actOpenDialog, &QAction::triggered, this, [this]() {
            QDialog dlg(this);
            dlg.setObjectName("dlgSampleDialog");
            dlg.setWindowTitle("Sample Dialog");

            auto* layout = new QVBoxLayout(&dlg);

            auto* lbl = new QLabel("This is a modal dialog.", &dlg);
            lbl->setObjectName("lblDialogText");
            layout->addWidget(lbl);

            auto* edit = new QLineEdit(&dlg);
            edit->setObjectName("leDialogInput");
            edit->setPlaceholderText("Type something...");
            layout->addWidget(edit);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            buttons->setObjectName("dbbDialogButtons");
            connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            layout->addWidget(buttons);

            dlg.exec();
        });

        auto* menuFile = menuBar()->addMenu("File");
        menuFile->setObjectName("menuFile");
        menuFile->addAction(actOpenDialog);

        auto* menuHelp = menuBar()->addMenu("Help");
        menuHelp->setObjectName("menuHelp");
        menuHelp->addAction(actAbout);

        auto* tb = addToolBar("Main");
        tb->setObjectName("tbMain");
        tb->addAction(actOpenDialog);
        tb->addAction(actAbout);

        auto* sb = statusBar();
        sb->setObjectName("sbStatusBar");
        sb->showMessage("Ready");
    }

    void setupDock()
    {
        auto* dock = new QDockWidget("Dock: List", this);
        dock->setObjectName("dockLeft");
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

        auto* list = new QListWidget(dock);
        list->setObjectName("lwDockList");
        for (int i = 0; i < 10; ++i) {
            list->addItem(QString("Dock Item %1").arg(i));
        }
        dock->setWidget(list);
        addDockWidget(Qt::LeftDockWidgetArea, dock);
    }

    QWidget* createFormTab()
    {
        auto* w = new QWidget;
        w->setObjectName("tabForm");

        auto* root = new QVBoxLayout(w);

        // 入力系
        auto* gbInput = new QGroupBox("Inputs");
        gbInput->setObjectName("gbInputs");
        auto* form = new QFormLayout(gbInput);

        auto* le = new QLineEdit;
        le->setObjectName("leName");
        le->setPlaceholderText("Your name");
        form->addRow("LineEdit:", le);

        auto* cb = new QComboBox;
        cb->setObjectName("cbChoice");
        cb->addItems({"Alpha", "Beta", "Gamma"});
        form->addRow("ComboBox:", cb);

        auto* sb = new QSpinBox;
        sb->setObjectName("sbCount");
        sb->setRange(0, 999);
        form->addRow("SpinBox:", sb);

        auto* dsb = new QDoubleSpinBox;
        dsb->setObjectName("dsbRatio");
        dsb->setRange(-100.0, 100.0);
        dsb->setDecimals(2);
        form->addRow("DoubleSpinBox:", dsb);

        auto* de = new QDateEdit;
        de->setObjectName("deDate");
        de->setCalendarPopup(true);
        form->addRow("DateEdit:", de);

        root->addWidget(gbInput);

        // 選択系
        auto* gbSelect = new QGroupBox("Selections");
        gbSelect->setObjectName("gbSelections");
        auto* vsel = new QVBoxLayout(gbSelect);

        auto* chk = new QCheckBox("CheckBox");
        chk->setObjectName("chkOption");
        vsel->addWidget(chk);

        auto* rb1 = new QRadioButton("Radio A");
        rb1->setObjectName("rbA");
        auto* rb2 = new QRadioButton("Radio B");
        rb2->setObjectName("rbB");
        rb1->setChecked(true);
        vsel->addWidget(rb1);
        vsel->addWidget(rb2);

        auto* slider = new QSlider(Qt::Horizontal);
        slider->setObjectName("slValue");
        slider->setRange(0, 100);
        slider->setValue(30);
        vsel->addWidget(new QLabel("Slider:"));
        vsel->addWidget(slider);

        m_progressBar = new QProgressBar;
        m_progressBar->setObjectName("pbProgress");
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);

        m_lcd = new QLCDNumber;
        m_lcd->setObjectName("lcdNumber");
        m_lcd->setDigitCount(3);
        m_lcd->display(0);

        auto* hl = new QHBoxLayout;
        hl->addWidget(m_progressBar);
        hl->addWidget(m_lcd);
        vsel->addLayout(hl);

        root->addWidget(gbSelect);

        // ボタン
        auto* hb = new QHBoxLayout;
        auto* btn1 = new QPushButton("Primary");
        btn1->setObjectName("btnPrimary");
        auto* btn2 = new QPushButton("Secondary");
        btn2->setObjectName("btnSecondary");
        hb->addWidget(btn1);
        hb->addWidget(btn2);
        hb->addStretch(1);
        root->addLayout(hb);

        // イベント例
        connect(btn1, &QPushButton::clicked, this, [this]() {
            statusBar()->showMessage("Primary clicked", 1500);
        });
        connect(btn2, &QPushButton::clicked, this, [this]() {
            statusBar()->showMessage("Secondary clicked", 1500);
        });

        root->addStretch(1);
        return w;
    }

    QWidget* createTextTab()
    {
        auto* w = new QWidget;
        w->setObjectName("tabText");

        auto* layout = new QHBoxLayout(w);

        auto* te = new QTextEdit;
        te->setObjectName("teRich");
        te->setPlainText("QTextEdit: rich text area\n- Type here\n- Paste here");
        layout->addWidget(te, 1);

        auto* pte = new QPlainTextEdit;
        pte->setObjectName("ptePlain");
        pte->setPlainText("QPlainTextEdit: plain text\nline 1\nline 2\nline 3");
        layout->addWidget(pte, 1);

        return w;
    }

    QWidget* createViewsTab()
    {
        auto* w = new QWidget;
        w->setObjectName("tabViews");

        auto* splitter = new QSplitter(Qt::Horizontal, w);
        splitter->setObjectName("spViews");

        // TableView
        auto* tv = new QTableView(splitter);
        tv->setObjectName("tvTable");
        tv->setModel(createTableModel(tv));
        tv->horizontalHeader()->setStretchLastSection(true);
        tv->setSelectionBehavior(QAbstractItemView::SelectRows);
        tv->setSelectionMode(QAbstractItemView::SingleSelection);
        tv->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

        // TreeView
        auto* trv = new QTreeView(splitter);
        trv->setObjectName("trvTree");
        trv->setModel(createTreeModel(trv));
        trv->expandAll();
        trv->header()->setStretchLastSection(true);

        // ListWidget
        auto* lw = new QListWidget(splitter);
        lw->setObjectName("lwList");
        for (int i = 0; i < 20; ++i) {
            lw->addItem(QString("List Item %1").arg(i));
        }

        splitter->setSizes({450, 350, 250});

        auto* root = new QVBoxLayout(w);
        root->addWidget(splitter);

        return w;
    }

    QWidget* createCalendarTab()
    {
        auto* w = new QWidget;
        w->setObjectName("tabCalendar");

        auto* layout = new QVBoxLayout(w);
        auto* cal = new QCalendarWidget;
        cal->setObjectName("cwCalendar");
        layout->addWidget(cal);

        auto* lbl = new QLabel("Pick a date in the calendar.");
        lbl->setObjectName("lblCalendarHint");
        layout->addWidget(lbl);

        connect(cal, &QCalendarWidget::selectionChanged, this, [cal, lbl]() {
            lbl->setText(QString("Selected: %1").arg(cal->selectedDate().toString(Qt::ISODate)));
        });

        return w;
    }

    void setupCentral()
    {
        auto* central = new QWidget;
        central->setObjectName("wCentral");
        setCentralWidget(central);

        auto* tabs = new QTabWidget;
        tabs->setObjectName("twMainTabs");

        tabs->addTab(createFormTab(), "Form");
        tabs->addTab(createTextTab(), "Text");
        tabs->addTab(createViewsTab(), "Views");
        tabs->addTab(createCalendarTab(), "Calendar");

        auto* layout = new QVBoxLayout(central);
        auto* topLabel = new QLabel("This window contains common Qt Widgets for GUI test verification.");
        topLabel->setObjectName("lblTop");
        layout->addWidget(topLabel);
        layout->addWidget(tabs, 1);
    }
};

MainWindow* make_allstars()
{
    auto window = new MainWindow{};
    window->show();
    window->raise();
    window->activateWindow();

    return window;
}

auto allstars = make_allstars();
