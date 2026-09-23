#pragma once

#include <QWidget>

class MessageWidget : public QWidget
{
public:
    explicit MessageWidget(const QString& message, QWidget* parent = nullptr);
};
