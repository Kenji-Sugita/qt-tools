#pragma once

#include <QWidget>

class MessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MessageWidget(const QString& message, QWidget* parent = nullptr);

signals:
    void quit();
};
