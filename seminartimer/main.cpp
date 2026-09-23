#include <QtWidgets>
#include <algorithm>

class TimerWidget : public QWidget
{
public:
    TimerWidget()
    {
        constexpr int windowSize = 392;
        setFixedSize(windowSize, windowSize);
        loadSettings();
        applyAlwaysOnTop();
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_NoSystemBackground);

        m_tick.setInterval(100);
        connect(&m_tick, &QTimer::timeout, this, [this] { onTick(); });

        m_flash.setInterval(180);
        connect(&m_flash, &QTimer::timeout, this, [this] {
            m_flashOn = !m_flashOn;
            update();
        });

        setMinutes(5);
        createTimeDialog();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        constexpr qreal outerMargin = 18.0;
        constexpr qreal shadowDx = 6.7;
        constexpr qreal shadowDy = 10.7;

        const qreal side = std::min(width() - (outerMargin * 2.0), height() - (outerMargin * 2.0) - shadowDy);
        const QPointF center(width() / 2.0, height() / 2.0 - shadowDy / 2.0);
        const QRectF panelRect(center.x() - side / 2.0, center.y() - side / 2.0, side, side);
        const QRectF ringRect = panelRect.adjusted(20, 20, -20, -20);
        const QRectF innerRect = ringRect.adjusted(26, 26, -26, -26);

        const double progress = m_totalMs > 0 ? std::clamp(double(m_remainingMs) / double(m_totalMs), 0.0, 1.0) : 0.0;
        const bool warning = m_running && m_remainingMs <= 10'000;
        const bool breakMode = m_breakMode;
        const QColor accentA = warning ? QColor(255, 89, 94) : (breakMode ? QColor(88, 170, 255) : QColor(0, 224, 255));
        const QColor accentB = warning ? QColor(255, 200, 87) : (breakMode ? QColor(120, 232, 255) : QColor(126, 255, 176));
        const QColor pulse = warning && m_flashOn ? QColor(255, 255, 255, 90) : QColor(255, 255, 255, 25);

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);

        QPainterPath shadowPath;
        shadowPath.addEllipse(panelRect.adjusted(shadowDx, shadowDy, shadowDx, shadowDy));
        p.fillPath(shadowPath, QColor(0, 0, 0, 80));

        QRadialGradient bg(center, side / 2.0);
        bg.setColorAt(0.0, warning ? QColor(72, 19, 30, 245) : (breakMode ? QColor(18, 31, 56, 245) : QColor(14, 23, 43, 245)));
        bg.setColorAt(0.7, warning ? QColor(39, 8, 14, 240) : (breakMode ? QColor(7, 16, 34, 240) : QColor(5, 11, 24, 240)));
        bg.setColorAt(1.0, QColor(0, 0, 0, 220));
        p.setBrush(bg);
        p.drawEllipse(panelRect);

        p.setBrush(pulse);
        p.drawEllipse(panelRect.adjusted(4, 4, -4, -4));

        QPen trackPen(QColor(255, 255, 255, 30), 18, Qt::SolidLine, Qt::RoundCap);
        p.setPen(trackPen);
        p.drawArc(ringRect, 0, 360 * 16);

        QConicalGradient ringGradient(center, 90);
        ringGradient.setColorAt(0.0, accentA);
        ringGradient.setColorAt(0.5, accentB);
        ringGradient.setColorAt(1.0, accentA);
        QPen progressPen(QBrush(ringGradient), 18, Qt::SolidLine, Qt::RoundCap);
        p.setPen(progressPen);
        p.drawArc(ringRect, 90 * 16, int(-360.0 * progress * 16.0));

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 255, 255, 10));
        p.drawEllipse(innerRect);

        QFont labelFont("Avenir Next", 12, QFont::DemiBold);
        labelFont.setLetterSpacing(QFont::AbsoluteSpacing, 2.2);
        p.setFont(labelFont);
        p.setPen(QColor(255, 255, 255, 180));
        p.drawText(QRectF(innerRect.left(), innerRect.top() + 20, innerRect.width(), 24), Qt::AlignCenter, "SEMINAR TIMER");

        if (breakMode) {
            drawBadge(p, QRectF(innerRect.center().x() - 64, innerRect.top() + 52, 128, 24), "BREAK MODE");
        }

        const int timeFontSize = 81;
        const qreal timeHeight = 96;
        const qreal timeTop = innerRect.center().y() - (timeHeight / 2.0);
        QFont timeFont("Avenir Next Condensed", timeFontSize, QFont::Bold);
        p.setFont(timeFont);
        p.setPen(Qt::white);
        p.drawText(QRectF(innerRect.left(), timeTop, innerRect.width(), timeHeight), Qt::AlignCenter, timeText());

        const QString stateText = warning ? "WRAP UP" : (m_running ? (breakMode ? "ON BREAK" : "IN PRACTICE") : "READY");
        QFont subFont("Avenir Next", 26, QFont::Medium);
        p.setFont(subFont);
        p.setPen(warning ? QColor(255, 220, 170) : (breakMode ? QColor(182, 225, 255) : QColor(175, 248, 255)));
        p.drawText(QRectF(innerRect.left(), timeTop + timeHeight + 18, innerRect.width(), 26), Qt::AlignCenter, stateText);

        drawSecondaryButton(p, m_startRect, m_running ? "PAUSE" : "START");
        drawSecondaryButton(p, m_resetRect, "RESET");
        drawChip(p, m_preset5Rect, "5", m_presetMinutes == 5);
        drawChip(p, m_preset10Rect, "10", m_presetMinutes == 10);
        drawChip(p, m_preset20Rect, "20", m_presetMinutes == 20);
        drawChip(p, m_preset30Rect, "30", m_presetMinutes == 30);
        drawChip(p, m_customRect, "SET", m_customSelected);
        drawChip(p, m_breakRect, "BREAK", m_breakMode);
    }

    void resizeEvent(QResizeEvent *) override
    {
        const int buttonY = height() - 82;
        m_resetRect = QRect(52, buttonY, 122, 40);
        m_startRect = QRect(width() - 174, buttonY, 122, 40);

        const int chipY = 40;
        const int chipGap = 6;
        const int chipGroupWidth = 50 + chipGap + 50 + chipGap + 50 + chipGap + 50 + chipGap + 58 + chipGap + 72;
        const int chipStartX = (width() - chipGroupWidth) / 2;
        m_preset5Rect = QRect(chipStartX, chipY, 50, 28);
        m_preset10Rect = QRect(m_preset5Rect.right() + chipGap + 1, chipY, 50, 28);
        m_preset20Rect = QRect(m_preset10Rect.right() + chipGap + 1, chipY, 50, 28);
        m_preset30Rect = QRect(m_preset20Rect.right() + chipGap + 1, chipY, 50, 28);
        m_customRect = QRect(m_preset30Rect.right() + chipGap + 1, chipY, 58, 28);
        m_breakRect = QRect(m_customRect.right() + chipGap + 1, chipY, 72, 28);
        positionTimeDialog();
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() != Qt::LeftButton) {
            return;
        }

        if (m_timeDialog && m_timeDialog->isVisible() && !m_timeDialog->geometry().contains(event->position().toPoint())) {
            m_timeDialog->hide();
            update();
        }

        if (m_startRect.contains(event->position().toPoint())) {
            toggleRunning();
            return;
        }
        if (m_resetRect.contains(event->position().toPoint())) {
            resetTimer();
            return;
        }
        if (m_preset5Rect.contains(event->position().toPoint())) {
            setMinutes(5);
            return;
        }
        if (m_preset10Rect.contains(event->position().toPoint())) {
            setMinutes(10);
            return;
        }
        if (m_preset20Rect.contains(event->position().toPoint())) {
            setMinutes(20);
            return;
        }
        if (m_preset30Rect.contains(event->position().toPoint())) {
            setMinutes(30);
            return;
        }
        if (m_customRect.contains(event->position().toPoint())) {
            openTimeDialog();
            return;
        }
        if (m_breakRect.contains(event->position().toPoint())) {
            toggleBreakMode();
            return;
        }

        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        m_dragging = true;
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (m_dragging && (event->buttons() & Qt::LeftButton)) {
            move(event->globalPosition().toPoint() - m_dragOffset);
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        QWidget::mouseReleaseEvent(event);
        m_dragging = false;
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        switch (event->key()) {
        case Qt::Key_Space:
            toggleRunning();
            break;
        case Qt::Key_R:
            resetTimer();
            break;
        case Qt::Key_Escape:
            if (m_timeDialog && m_timeDialog->isVisible()) {
                m_timeDialog->hide();
            } else {
                close();
            }
            break;
        case Qt::Key_0:
            setMinutes(5);
            break;
        case Qt::Key_1:
            setMinutes(10);
            break;
        case Qt::Key_2:
            setMinutes(20);
            break;
        case Qt::Key_3:
            setMinutes(30);
            break;
        case Qt::Key_T:
            openTimeDialog();
            break;
        case Qt::Key_B:
            toggleBreakMode();
            break;
        default:
            QWidget::keyPressEvent(event);
            return;
        }
        update();
    }

private:
    void drawSecondaryButton(QPainter &p, const QRect &rect, const QString &text)
    {
        p.setPen(QPen(QColor(255, 255, 255, 130), 1.4));
        p.setBrush(QColor(70, 78, 96, 210));
        p.drawRoundedRect(rect, 10, 10);

        p.setPen(QColor(255, 255, 255, 250));
        QFont font("Avenir Next", 22, QFont::DemiBold);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
        p.setFont(font);
        p.drawText(rect, Qt::AlignCenter, text);
    }

    void drawBadge(QPainter &p, const QRectF &rect, const QString &text)
    {
        p.setPen(QPen(QColor(150, 219, 255, 110), 1.0));
        p.setBrush(QColor(73, 127, 193, 72));
        p.drawRoundedRect(rect, 12, 12);

        p.setPen(QColor(218, 242, 255, 220));
        QFont font("Avenir Next", 10, QFont::DemiBold);
        font.setLetterSpacing(QFont::AbsoluteSpacing, 1.6);
        p.setFont(font);
        p.drawText(rect, Qt::AlignCenter, text);
    }

    void drawChip(QPainter &p, const QRect &rect, const QString &text, bool selected)
    {
        p.setPen(Qt::NoPen);
        p.setBrush(selected ? QColor(255, 255, 255, 48) : QColor(255, 255, 255, 16));
        p.drawRoundedRect(rect, 14, 14);

        p.setPen(selected ? QColor(255, 255, 255) : QColor(255, 255, 255, 180));
        QFont font("Avenir Next", 12, QFont::Bold);
        p.setFont(font);
        const QString label = (text == "SET" || text == "BREAK") ? text : text + " min";
        p.drawText(rect, Qt::AlignCenter, label);
    }

    QString timeText() const
    {
        const int totalSeconds = std::max(0, m_remainingMs / 1000);
        const int minutes = totalSeconds / 60;
        const int seconds = totalSeconds % 60;
        return QStringLiteral("%1:%2")
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }

    void setMinutes(int minutes)
    {
        setDuration(minutes * 60 * 1000, false);
    }

    void resetTimer()
    {
        setDuration(m_presetMs, m_customSelected);
    }

    void setDuration(int durationMs, bool customSelected)
    {
        m_presetMs = durationMs;
        m_totalMs = durationMs;
        m_remainingMs = m_totalMs;
        m_running = false;
        m_tick.stop();
        m_flash.stop();
        m_flashOn = false;
        m_warningBeeped = false;
        m_finishedBeeped = false;
        m_customSelected = customSelected;
        m_presetMinutes = customSelected ? -1 : durationMs / (60 * 1000);
        update();
    }

    void openTimeDialog()
    {
        const int totalMinutes = std::max(0, m_presetMs / (60 * 1000));
        m_minutesSpin->setValue(totalMinutes);
        m_alwaysOnTopCheck->setChecked(m_alwaysOnTop);
        positionTimeDialog();
        m_timeDialog->show();
        m_timeDialog->raise();
        m_minutesSpin->setFocus();
        update();
    }

    void createTimeDialog()
    {
        m_timeDialog = new QFrame(this);
        m_timeDialog->setObjectName("timeDialog");
        m_timeDialog->setStyleSheet(
            "#timeDialog { background: rgba(16, 22, 36, 235); border: 1.4px solid rgba(255,255,255,0.51); border-radius: 18px; }"
            "#timeDialog QLabel { color: rgba(255,255,255,0.92); font: 600 20px 'Avenir Next'; }"
            "#timeDialog QCheckBox { color: rgba(255,255,255,0.92); spacing: 8px; font: 600 18px 'Avenir Next'; }"
            "#timeDialog QCheckBox::indicator { width: 18px; height: 18px; }"
            "#timeDialog QCheckBox::indicator:unchecked { border: 1px solid rgba(255,255,255,0.45); background: rgba(255,255,255,0.08); }"
            "#timeDialog QCheckBox::indicator:checked { border: 1px solid rgba(117,228,235,0.9); background: rgba(117,228,235,0.88); }"
            "#timeDialog QWidget#spinRow { background: rgba(255,255,255,0.08); border: 1px solid rgba(255,255,255,0.18); border-radius: 10px; }"
            "#timeDialog QSpinBox { min-width: 88px; max-width: 88px; padding: 6px 0; color: white; background: transparent; border: none; font: 600 40px 'Avenir Next'; selection-background-color: transparent; selection-color: white; }"
            "#timeDialog QToolButton { min-width: 40px; max-width: 40px; min-height: 40px; max-height: 40px; color: rgba(255,255,255,0.98); background: rgba(117, 228, 235, 0.88); border: none; font: 600 32px 'Avenir Next'; }"
            "#timeDialog QToolButton#decrementButton { border-top-left-radius: 10px; border-bottom-left-radius: 10px; border-top: 1px solid rgba(255,255,255,0.18); border-bottom: 1px solid rgba(255,255,255,0.18); border-left: 1px solid rgba(255,255,255,0.18); border-right: 1px solid rgba(255,255,255,0.18); }"
            "#timeDialog QToolButton#incrementButton { border-top-right-radius: 10px; border-bottom-right-radius: 10px; border-top: 1px solid rgba(255,255,255,0.18); border-bottom: 1px solid rgba(255,255,255,0.18); border-left: 1px solid rgba(255,255,255,0.18); border-right: 1px solid rgba(255,255,255,0.18); }"
            "#timeDialog QPushButton { min-width: 122px; max-width: 122px; max-height: 40px; min-height: 40px; padding: 0; color: rgba(255,255,255,0.98); background: rgba(70,78,96,210); border: 1px solid rgba(255,255,255,0.18); border-radius: 10px; font: 600 24px 'Avenir Next'; }"
            "#timeDialog QPushButton#cancelButton { background: rgba(70,78,96,210); border: 1px solid rgba(255,255,255,0.18); }"
        );
        m_timeDialog->hide();

        auto *layout = new QVBoxLayout(m_timeDialog);
        auto *label = new QLabel("Set Practice Time", m_timeDialog);
        auto *form = new QFormLayout;
        auto *minutesLabel = new QLabel("Minutes", m_timeDialog);
        m_minutesSpin = new QSpinBox(m_timeDialog);
        auto *spinRow = new QWidget(m_timeDialog);
        auto *spinLayout = new QHBoxLayout(spinRow);
        auto *decrementButton = new QToolButton(spinRow);
        auto *incrementButton = new QToolButton(spinRow);
        auto *buttonsLayout = new QHBoxLayout;
        auto *okButton = new QPushButton("OK", m_timeDialog);
        auto *cancelButton = new QPushButton("Cancel", m_timeDialog);
        m_alwaysOnTopCheck = new QCheckBox("Always on top", m_timeDialog);

        m_minutesSpin->setRange(1, 180);
        m_minutesSpin->setButtonSymbols(QAbstractSpinBox::NoButtons);
        m_minutesSpin->setAlignment(Qt::AlignRight);
        m_minutesSpin->setReadOnly(true);
        if (auto *spinEdit = m_minutesSpin->findChild<QLineEdit *>()) {
            spinEdit->setReadOnly(true);
            spinEdit->setFocusPolicy(Qt::NoFocus);
        }
        spinRow->setObjectName("spinRow");
        cancelButton->setObjectName("cancelButton");
        decrementButton->setObjectName("decrementButton");
        incrementButton->setObjectName("incrementButton");
        decrementButton->setText("-");
        incrementButton->setText("+");
        okButton->setFixedSize(122, 40);
        cancelButton->setFixedSize(122, 40);

        spinLayout->setContentsMargins(0, 0, 0, 0);
        spinLayout->setSpacing(0);
        spinLayout->addWidget(decrementButton);
        spinLayout->addWidget(m_minutesSpin);
        spinLayout->addWidget(incrementButton);

        QFont buttonFont("Avenir Next", 22, QFont::DemiBold);
        buttonFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
        okButton->setFont(buttonFont);
        cancelButton->setFont(buttonFont);

        QFont fieldLabelFont("Avenir Next", 20, QFont::DemiBold);
        minutesLabel->setFont(fieldLabelFont);
        minutesLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        minutesLabel->setFixedHeight(spinRow->sizeHint().height());

        QFont spinValueFont("Avenir Next", 26, QFont::DemiBold);
        m_minutesSpin->setFont(spinValueFont);

        form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
        form->setFormAlignment(Qt::AlignHCenter);
        form->setVerticalSpacing(12);
        form->addRow(minutesLabel, spinRow);

        buttonsLayout->addWidget(cancelButton);
        buttonsLayout->addWidget(okButton);

        layout->addWidget(label);
        layout->addLayout(form);
        layout->addWidget(m_alwaysOnTopCheck, 0, Qt::AlignCenter);
        layout->addLayout(buttonsLayout);

        connect(okButton, &QPushButton::clicked, this, [this] { applyDialogTime(); });
        connect(cancelButton, &QPushButton::clicked, this, [this] {
            m_timeDialog->hide();
            update();
        });
        connect(decrementButton, &QToolButton::clicked, m_minutesSpin, qOverload<>(&QSpinBox::stepDown));
        connect(incrementButton, &QToolButton::clicked, m_minutesSpin, qOverload<>(&QSpinBox::stepUp));
    }

    void positionTimeDialog()
    {
        if (!m_timeDialog) {
            return;
        }

        m_timeDialog->adjustSize();
        const QPoint center((width() - m_timeDialog->width()) / 2, (height() - m_timeDialog->height()) / 2);
        m_timeDialog->move(center);
    }

    void applyDialogTime()
    {
        const int durationMs = m_minutesSpin->value() * 60 * 1000;
        if (durationMs > 0) {
            setDuration(durationMs, true);
        }
        const bool alwaysOnTop = m_alwaysOnTopCheck->isChecked();
        if (m_alwaysOnTop != alwaysOnTop) {
            m_alwaysOnTop = alwaysOnTop;
            applyAlwaysOnTop();
        }
        saveSettings();
        m_timeDialog->hide();
        update();
    }

    void applyAlwaysOnTop()
    {
        const QRect geometryBeforeChange = geometry();
        const bool wasVisible = isVisible();

        setAttribute(Qt::WA_ShowWithoutActivating, m_alwaysOnTop);
        if (m_alwaysOnTop) {
            setWindowFlags(Qt::FramelessWindowHint | Qt::Window | Qt::WindowStaysOnTopHint);
        } else {
            setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        }

        if (geometryBeforeChange.isValid()) {
            setGeometry(geometryBeforeChange);
        }

        if (wasVisible) {
            show();
            raise();
            if (!m_alwaysOnTop) {
                activateWindow();
            }
        }
    }

    void loadSettings()
    {
        QSettings settings;
        m_alwaysOnTop = settings.value(QStringLiteral("window/alwaysOnTop"), true).toBool();
    }

    void saveSettings() const
    {
        QSettings settings;
        settings.setValue(QStringLiteral("window/alwaysOnTop"), m_alwaysOnTop);
    }

    void toggleBreakMode()
    {
        m_breakMode = !m_breakMode;
        update();
    }

    void toggleRunning()
    {
        m_running = !m_running;
        if (m_running) {
            m_endTime = QDateTime::currentDateTime().addMSecs(m_remainingMs);
            m_warningBeeped = m_remainingMs <= 10'000;
            m_finishedBeeped = false;
            m_tick.start();
        } else {
            m_remainingMs = std::max(0, int(QDateTime::currentDateTime().msecsTo(m_endTime)));
            m_tick.stop();
            m_flash.stop();
            m_flashOn = false;
        }
        update();
    }

    void onTick()
    {
        m_remainingMs = std::max(0, int(QDateTime::currentDateTime().msecsTo(m_endTime)));
        if (m_running && m_remainingMs <= 10'000 && !m_warningBeeped) {
            playAlert(1);
            m_warningBeeped = true;
        }
        if (m_remainingMs <= 10'000) {
            if (!m_flash.isActive()) {
                m_flash.start();
            }
        } else {
            m_flash.stop();
            m_flashOn = false;
        }

        if (m_remainingMs == 0) {
            m_running = false;
            m_tick.stop();
            m_flash.stop();
            m_flashOn = true;
            if (!m_finishedBeeped) {
                playAlert(3);
                m_finishedBeeped = true;
            }
        }
        update();
    }

    void playAlert(int count)
    {
        for (int i = 0; i < count; ++i) {
            QTimer::singleShot(i * 180, this, [] { QApplication::beep(); });
        }
    }

    QTimer m_tick;
    QTimer m_flash;
    QDateTime m_endTime;
    int m_totalMs = 0;
    int m_remainingMs = 0;
    int m_presetMinutes = 5;
    int m_presetMs = 5 * 60 * 1000;
    bool m_running = false;
    bool m_flashOn = false;
    bool m_warningBeeped = false;
    bool m_finishedBeeped = false;
    bool m_customSelected = false;
    bool m_breakMode = false;
    bool m_alwaysOnTop = true;
    bool m_dragging = false;
    QPoint m_dragOffset;
    QRect m_startRect;
    QRect m_resetRect;
    QRect m_preset5Rect;
    QRect m_preset10Rect;
    QRect m_preset20Rect;
    QRect m_preset30Rect;
    QRect m_customRect;
    QRect m_breakRect;
    QFrame *m_timeDialog = nullptr;
    QCheckBox *m_alwaysOnTopCheck = nullptr;
    QSpinBox *m_minutesSpin = nullptr;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SeminarTimer");
    app.setOrganizationName("sugita");
    app.setQuitOnLastWindowClosed(true);

    TimerWidget widget;
    const QRect screen = widget.screen() ? widget.screen()->availableGeometry() : QRect(0, 0, 1280, 800);
    widget.move(screen.right() - widget.width() - 40, screen.top() + 40);
    widget.show();

    return app.exec();
}
