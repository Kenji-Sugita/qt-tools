#include "ImageExportDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QVBoxLayout>

ImageExportDialog::ImageExportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Image Export Settings"));
    setModal(true);

    m_formatComboBox = new QComboBox(this);
    m_formatComboBox->addItem(tr("PNG"), static_cast<int>(ImageExporter::Format::Png));
    m_formatComboBox->addItem(tr("SVG"), static_cast<int>(ImageExporter::Format::Svg));

    m_transparentBackgroundButton = new QRadioButton(tr("Transparent"), this);
    m_whiteBackgroundButton = new QRadioButton(tr("White"), this);
    m_transparentBackgroundButton->setChecked(true);

    auto *backgroundLayout = new QVBoxLayout;
    backgroundLayout->addWidget(m_transparentBackgroundButton);
    backgroundLayout->addWidget(m_whiteBackgroundButton);
    auto *backgroundGroup = new QGroupBox(tr("Background"), this);
    backgroundGroup->setLayout(backgroundLayout);

    auto *formLayout = new QFormLayout;
    formLayout->addRow(tr("Format:"), m_formatComboBox);
    formLayout->addRow(backgroundGroup);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *layout = new QVBoxLayout(this);
    layout->addLayout(formLayout);
    layout->addWidget(buttons);
    setLayout(layout);
}

ImageExporter::Format ImageExportDialog::format() const
{
    return static_cast<ImageExporter::Format>(m_formatComboBox->currentData().toInt());
}

ImageExporter::Background ImageExportDialog::background() const
{
    return m_whiteBackgroundButton->isChecked()
        ? ImageExporter::Background::White : ImageExporter::Background::Transparent;
}
