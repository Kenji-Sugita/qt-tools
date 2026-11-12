#include "ImageExportDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QRadioButton>
#include <QTest>

class ImageExportDialogTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsAndControls();
};

void ImageExportDialogTest::defaultsAndControls()
{
    ImageExportDialog dialog;
    QCOMPARE(dialog.format(), ImageExporter::Format::Png);
    QCOMPARE(dialog.background(), ImageExporter::Background::Transparent);

    auto *formatComboBox = dialog.findChild<QComboBox *>();
    QVERIFY(formatComboBox);
    QCOMPARE(formatComboBox->count(), 2);
    formatComboBox->setCurrentIndex(1);
    QCOMPARE(dialog.format(), ImageExporter::Format::Svg);

    const QList<QRadioButton *> backgroundButtons = dialog.findChildren<QRadioButton *>();
    QCOMPARE(backgroundButtons.size(), 2);
    backgroundButtons.at(1)->setChecked(true);
    QCOMPARE(dialog.background(), ImageExporter::Background::White);

    auto *buttons = dialog.findChild<QDialogButtonBox *>();
    QVERIFY(buttons);
    QVERIFY(buttons->button(QDialogButtonBox::Save));
    QVERIFY(buttons->button(QDialogButtonBox::Cancel));
}

QTEST_MAIN(ImageExportDialogTest)

#include "tst_imageexportdialog.moc"
