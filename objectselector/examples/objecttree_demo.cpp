#include "objecthighlighter.h"
#include "objecttreewidget.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace {

QWidget *createPreviewWidget()
{
    auto *previewWidget = new QWidget;
    auto *rootLayout = new QVBoxLayout(previewWidget);

    auto *titleLabel = new QLabel(QStringLiteral("ObjectTreeWidget Demo"), previewWidget);
    titleLabel->setObjectName(QStringLiteral("titleLabel"));

    auto *formGroupBox = new QGroupBox(QStringLiteral("Controls"), previewWidget);
    formGroupBox->setObjectName(QStringLiteral("controlsGroupBox"));
    auto *formLayout = new QVBoxLayout(formGroupBox);

    auto *nameLineEdit = new QLineEdit(QStringLiteral("Sample Text"), formGroupBox);
    nameLineEdit->setObjectName(QStringLiteral("nameLineEdit"));

    auto *enabledCheckBox = new QCheckBox(QStringLiteral("Enabled"), formGroupBox);
    enabledCheckBox->setObjectName(QStringLiteral("enabledCheckBox"));
    enabledCheckBox->setChecked(true);

    auto *valueSlider = new QSlider(Qt::Horizontal, formGroupBox);
    valueSlider->setObjectName(QStringLiteral("valueSlider"));
    valueSlider->setValue(42);

    auto *valueSpinBox = new QSpinBox(formGroupBox);
    valueSpinBox->setObjectName(QStringLiteral("valueSpinBox"));
    valueSpinBox->setValue(42);
    QObject::connect(valueSlider, &QSlider::valueChanged, valueSpinBox, &QSpinBox::setValue);
    QObject::connect(valueSpinBox, qOverload<int>(&QSpinBox::valueChanged), valueSlider, &QSlider::setValue);

    auto *buttonRowWidget = new QWidget(formGroupBox);
    buttonRowWidget->setObjectName(QStringLiteral("buttonRowWidget"));
    auto *buttonRowLayout = new QHBoxLayout(buttonRowWidget);
    auto *applyButton = new QPushButton(QStringLiteral("Apply"), buttonRowWidget);
    applyButton->setObjectName(QStringLiteral("applyButton"));
    auto *resetButton = new QPushButton(QStringLiteral("Reset"), buttonRowWidget);
    resetButton->setObjectName(QStringLiteral("resetButton"));
    buttonRowLayout->addWidget(applyButton);
    buttonRowLayout->addWidget(resetButton);

    formLayout->addWidget(nameLineEdit);
    formLayout->addWidget(enabledCheckBox);
    formLayout->addWidget(valueSlider);
    formLayout->addWidget(valueSpinBox);
    formLayout->addWidget(buttonRowWidget);

    auto *notesEdit = new QTextEdit(previewWidget);
    notesEdit->setObjectName(QStringLiteral("notesEdit"));
    notesEdit->setPlainText(QStringLiteral("Select objects in the tree to inspect and highlight them."));

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(formGroupBox);
    rootLayout->addWidget(notesEdit);

    return previewWidget;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("ObjectTreeWidget Standalone Demo"));

    auto *splitter = new QSplitter(&window);
    auto *objectTreeWidget = new ObjectTreeWidget(splitter);
    auto *previewWidget = createPreviewWidget();
    splitter->addWidget(objectTreeWidget);
    splitter->addWidget(previewWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    ObjectHighlighter highlighter;
    QObject::connect(objectTreeWidget, &ObjectTreeWidget::objectActivated,
                     &window, [&highlighter](QObject *targetObject) {
                         highlighter.setTargetObject(targetObject);
                     });

    objectTreeWidget->setRootObject(previewWidget);
    objectTreeWidget->setCurrentObject(previewWidget);
    highlighter.setTargetObject(previewWidget);

    window.setCentralWidget(splitter);
    window.resize(1100, 700);
    window.show();
    return app.exec();
}
