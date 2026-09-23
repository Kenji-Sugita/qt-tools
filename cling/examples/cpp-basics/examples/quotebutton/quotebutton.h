#ifndef QUOTEBUTTON_H
#define QUOTEBUTTON_H

#include <QPushButton>
#include <QStringList>

class QuoteButton :public QPushButton
{
    Q_OBJECT

public:
    explicit QuoteButton(const QString& text, QWidget* parent = nullptr);

signals:
    void quote(const QString& message);

protected slots:
    void sendQuote();

private:
    int quoteNo;
    const QStringList quotes;
};
#endif
