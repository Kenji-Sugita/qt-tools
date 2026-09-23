// GitHub Copilot: Qt/C++ の icpp 用サンプルを生成してください。
// QLabel/QSlider/QPushButton に objectName を付け、PropertyEditor で確認しやすい Widget を作ってください。
// updatePreview() と go() 関数を用意し、main() は書かないでください。

#include "widget.h"

#include <QFont>
#include <QVBoxLayout>

Widget::Widget(QWidget *parent) : QWidget(parent) {
    setObjectName("inspectLessonWidget");
    setWindowTitle("12 Inspect Widget");

    titleLabel = new QLabel("Inspect this window", this);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);

    previewLabel = new QLabel("icpp", this);
    previewLabel->setObjectName("previewLabel");
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setMinimumHeight(80);

    sizeSlider = new QSlider(Qt::Horizontal, this);
    sizeSlider->setObjectName("sizeSlider");
    sizeSlider->setRange(18, 56);
    sizeSlider->setValue(32);

    accentButton = new QPushButton("Toggle accent", this);
    accentButton->setObjectName("accentButton");

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(titleLabel);
    layout->addWidget(previewLabel);
    layout->addWidget(sizeSlider);
    layout->addWidget(accentButton, 0, Qt::AlignCenter);

    QObject::connect(sizeSlider, &QSlider::valueChanged, this, [this]() {
        updatePreview();
    });
    QObject::connect(accentButton, &QPushButton::clicked, this, [this]() {
        accent = !accent;
        updatePreview();
    });

    updatePreview();
    resize(360, 220);
}

void Widget::updatePreview() {
    QFont font = previewLabel->font();
    font.setPointSize(sizeSlider->value());
    font.setBold(accent);
    previewLabel->setFont(font);
    previewLabel->setStyleSheet(accent ? "color: white; background: #2f6fbd; padding: 8px;"
                                       : "color: #222; background: #eeeeee; padding: 8px;");
}

Widget *go() {
    return new Widget;
}
