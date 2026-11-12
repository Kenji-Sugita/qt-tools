#include "objectpicker.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace {

QString describeObject(const QObject *targetObject)
{
    if (targetObject == nullptr) {
        return QStringLiteral("<none>");
    }

    const QString objectName = targetObject->objectName().isEmpty()
        ? QStringLiteral("<unnamed>")
        : targetObject->objectName();
    return QStringLiteral("%1 (%2)")
        .arg(objectName, QString::fromLatin1(targetObject->metaObject()->className()));
}

QWidget *createPreviewWidget()
{
    auto *previewWidget = new QWidget;
    previewWidget->setObjectName(QStringLiteral("previewRootWidget"));

    auto *rootLayout = new QVBoxLayout(previewWidget);

    auto *toolbarFrame = new QFrame(previewWidget);
    toolbarFrame->setObjectName(QStringLiteral("toolbarFrame"));
    auto *toolbarLayout = new QHBoxLayout(toolbarFrame);
    auto *searchLineEdit = new QLineEdit(QStringLiteral("Search"), toolbarFrame);
    searchLineEdit->setObjectName(QStringLiteral("searchLineEdit"));
    auto *filterCheckBox = new QCheckBox(QStringLiteral("Match Case"), toolbarFrame);
    filterCheckBox->setObjectName(QStringLiteral("filterCheckBox"));
    auto *refreshButton = new QPushButton(QStringLiteral("Refresh"), toolbarFrame);
    refreshButton->setObjectName(QStringLiteral("refreshButton"));
    toolbarLayout->addWidget(searchLineEdit);
    toolbarLayout->addWidget(filterCheckBox);
    toolbarLayout->addWidget(refreshButton);

    auto *contentGroupBox = new QGroupBox(QStringLiteral("Preview Panel"), previewWidget);
    contentGroupBox->setObjectName(QStringLiteral("contentGroupBox"));
    auto *contentLayout = new QVBoxLayout(contentGroupBox);
    auto *descriptionLabel = new QLabel(QStringLiteral("Use the picker button to select any widget in the preview."), contentGroupBox);
    descriptionLabel->setWordWrap(true);
    descriptionLabel->setObjectName(QStringLiteral("descriptionLabel"));
    auto *sizeSlider = new QSlider(Qt::Horizontal, contentGroupBox);
    sizeSlider->setObjectName(QStringLiteral("sizeSlider"));
    sizeSlider->setValue(60);
    auto *sizeSpinBox = new QSpinBox(contentGroupBox);
    sizeSpinBox->setObjectName(QStringLiteral("sizeSpinBox"));
    sizeSpinBox->setValue(60);
    QObject::connect(sizeSlider, &QSlider::valueChanged, sizeSpinBox, &QSpinBox::setValue);
    QObject::connect(sizeSpinBox, qOverload<int>(&QSpinBox::valueChanged), sizeSlider, &QSlider::setValue);
    auto *notesEdit = new QPlainTextEdit(contentGroupBox);
    notesEdit->setObjectName(QStringLiteral("notesEdit"));
    notesEdit->setPlainText(QStringLiteral("Picked objects stay highlighted after selection."));
    contentLayout->addWidget(descriptionLabel);
    contentLayout->addWidget(sizeSlider);
    contentLayout->addWidget(sizeSpinBox);
    contentLayout->addWidget(notesEdit);

    rootLayout->addWidget(toolbarFrame);
    rootLayout->addWidget(contentGroupBox);
    return previewWidget;
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("ObjectPicker Standalone Demo"));

    auto *centralWidget = new QWidget(&window);
    auto *mainLayout = new QVBoxLayout(centralWidget);

    auto *instructionLabel = new QLabel(
        QStringLiteral("Click \"Start Picker\", hover the preview, then click a widget to pick it."),
        centralWidget);
    instructionLabel->setWordWrap(true);

    auto *pickedObjectLabel = new QLabel(QStringLiteral("Current Pick: <none>"), centralWidget);
    pickedObjectLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto *buttonRowWidget = new QWidget(centralWidget);
    auto *buttonRowLayout = new QHBoxLayout(buttonRowWidget);
    auto *startPickerButton = new QPushButton(QStringLiteral("Start Picker"), centralWidget);
    auto *clearHighlightButton = new QPushButton(QStringLiteral("Clear Highlight"), centralWidget);
    buttonRowLayout->setContentsMargins(0, 0, 0, 0);
    buttonRowLayout->addWidget(startPickerButton);
    buttonRowLayout->addWidget(clearHighlightButton);
    buttonRowLayout->addStretch();

    auto *previewWidget = createPreviewWidget();

    mainLayout->addWidget(instructionLabel);
    mainLayout->addWidget(pickedObjectLabel);
    mainLayout->addWidget(buttonRowWidget);
    mainLayout->addWidget(previewWidget);

    ObjectPicker objectPicker;

    QObject::connect(startPickerButton, &QPushButton::clicked, &window, [&objectPicker, previewWidget]() {
        objectPicker.start(previewWidget);
    });
    QObject::connect(clearHighlightButton, &QPushButton::clicked, &window,
                     [&objectPicker, pickedObjectLabel]() {
                         objectPicker.setSelectedObject(nullptr);
                         pickedObjectLabel->setText(QStringLiteral("Current Pick: <none>"));
                     });
    QObject::connect(&objectPicker, &ObjectPicker::objectPicked, &window,
                     [&pickedObjectLabel](QObject *targetObject) {
                         pickedObjectLabel->setText(QStringLiteral("Current Pick: %1")
                                                        .arg(describeObject(targetObject)));
                     });

    window.setCentralWidget(centralWidget);
    window.resize(900, 620);
    window.show();
    return app.exec();
}
