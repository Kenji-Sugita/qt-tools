#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QFont>
#include <QFormLayout>
#include <QValidator>

class IPValidator : public QValidator
{
    Q_OBJECT

public:
    explicit IPValidator(QLineEdit* parent)
        : QValidator(parent) {
            Q_ASSERT(parent);
        }

protected:
    virtual State validate (QString& input, int& cursorPos) const {
        Q_UNUSED(cursorPos);
        // Range checking of each IP address components
        for (int pos = 0; pos < 15; pos += 4) {
            bool ok1;
            int i = input.mid(pos, 1).toInt(&ok1);
            bool ok2;
            int j = input.mid(pos+1, 1).toInt(&ok2);
            bool ok3;
            int k = input.mid(pos+2, 1).toInt(&ok3);

            if ((ok1 && i > 2) ||
		(ok1 && ok2 && i == 2 && j > 5) ||
                (ok1 && ok2 && ok3 && i*100 + j*10 + k > 255))
                return Invalid;
        }
        return Acceptable;
    }
};

QFont fixedFont()
{
    static QFont platformDependFont = []() {
        QFont aFont;
#if defined(Q_OS_MACOS)
        aFont = QFont("Osaka");
        aFont.setStyleName("Regular-Mono");
#elif defined(Q_OS_LINUX)
        aFont = QFont("DejaVuSansMono");
#elif defined(Q_OS_WIN)
        aFont = QFont("Lucida Console");
#endif
        return aFont;
    }();

    return platformDependFont;
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    const auto ipLabel = new QLabel("IP Address:");
    const auto ipEdit = new QLineEdit;

    ipEdit->setFont(fixedFont());
    ipEdit->setInputMask("000.000.000.000;_");
    ipEdit->setValidator(new IPValidator(ipEdit));

    const auto isoLabel = new QLabel("ISO Date:");
    const auto isoEdit = new QLineEdit;
    isoEdit->setFont(fixedFont());
    isoEdit->setInputMask("0000-00-00");

    const auto licenseLabel = new QLabel("License Number:");
    const auto licenseEdit = new QLineEdit;
    licenseEdit->setFont(fixedFont());
    licenseEdit->setInputMask(">AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;#");

    QWidget top;
    top.setMinimumWidth(450);

    const auto topLayout = new QFormLayout(&top);
    topLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    topLayout->addRow(ipLabel, ipEdit);
    topLayout->addRow(isoLabel, isoEdit);
    topLayout->addRow(licenseLabel, licenseEdit);

    top.show();

    return app.exec();
}

#include "main.moc"
