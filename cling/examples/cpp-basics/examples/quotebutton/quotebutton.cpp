#include "quotebutton.h"
//#include <QDebug>

QuoteButton::QuoteButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent),
      quoteNo(-1),
      quotes { "I'm a Michael Bolton fan. Could you play it for me?",
               "About your TPS reports...",
               "You will have to come in Saturday, will'ya?",
               "we're putting new coversheets on all the TPS reports *before* they go out now",
               "bear with me for just a second",
               "You see Bob, it's not that I'm lazy, it's that I just don't care.",
               "No, not again.",
               "I swear to God, one of these days...",
               "that's just a straight shooter with upper management written all over him",
               "Standard operating procedure.",
               "I could set the building on fire.",
               "Excuse me, I believe you have my stapler... ",
               "I could put... I could put... strychnine in the guacamole.",
               "What am I gonna do with 40 subscriptions to Vibe?",
               "Oh, and next Friday... is Hawaiian shirt day...",
               "You know, minimum security prison is no picnic.",
               "I have people skills.",
               "I can't believe what a bunch of nerds we are.",
               "The ratio of people to cake is too big." } {
    connect(this, &QuoteButton::clicked, this, &QuoteButton::sendQuote);
}

void QuoteButton::sendQuote()
{
//    qDebug() << Q_FUNC_INFO << "quoteNo =" << quoteNo;
    quoteNo = (quoteNo + 1) % quotes.size();
    emit quote(quotes[quoteNo]);
}
