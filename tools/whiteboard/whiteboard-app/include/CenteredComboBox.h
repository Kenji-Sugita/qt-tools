#pragma once

#include <QComboBox>

class CenteredComboBox : public QComboBox {
public:
    explicit CenteredComboBox(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};
