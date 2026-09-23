#pragma once

#include "ImageExporter.h"

#include <QDialog>

class QComboBox;
class QRadioButton;

class ImageExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImageExportDialog(QWidget *parent = nullptr);

    ImageExporter::Format format() const;
    ImageExporter::Background background() const;

private:
    QComboBox *m_formatComboBox = nullptr;
    QRadioButton *m_transparentBackgroundButton = nullptr;
    QRadioButton *m_whiteBackgroundButton = nullptr;
};
