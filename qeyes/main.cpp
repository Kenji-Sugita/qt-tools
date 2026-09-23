#include <QApplication>
#include <QCloseEvent>
#include <QCheckBox>
#include <QColorDialog>
#include <QCursor>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include <QWidget>
#include <QtMath>
#include <functional>

class EyesWidget : public QWidget
{
public:
    EyesWidget()
        : QWidget(nullptr, Qt::FramelessWindowHint | Qt::Window)
    {
        setWindowTitle(QStringLiteral("qeyes"));
        resize(300, 190);
        setAttribute(Qt::WA_TranslucentBackground, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        setAttribute(Qt::WA_MacAlwaysShowToolWindow, true);
        setAutoFillBackground(false);
        setWindowFlag(Qt::FramelessWindowHint, true);
        setWindowFlag(Qt::Window, true);
        setWindowFlag(Qt::NoDropShadowWindowHint, true);
        loadSettings();
        applyAlwaysOnTop();

        auto *timer = new QTimer(this);
        timer->setInterval(16);
        connect(timer, &QTimer::timeout, this, [this]() {
            update();
        });
        timer->start();

        connect(qApp, &QGuiApplication::applicationStateChanged, this,
                [this](Qt::ApplicationState state) {
                    if (state != Qt::ApplicationActive || m_appearance.alwaysOnTop)
                        return;

                    show();
                    raise();
                    activateWindow();
                });
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(rect(), Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

        const QRectF bounds = rect().adjusted(6, 6, -6, -6);
        const qreal eyeWidth = bounds.width() * 0.38;
        const qreal eyeHeight = bounds.height() * 0.92;
        const qreal gap = bounds.width() * 0.08;
        const qreal leftX = bounds.center().x() - gap * 0.5 - eyeWidth;
        const qreal rightX = bounds.center().x() + gap * 0.5;
        const qreal eyeY = bounds.center().y() - eyeHeight * 0.5;

        const QRectF leftEye(leftX, eyeY, eyeWidth, eyeHeight);
        const QRectF rightEye(rightX, eyeY, eyeWidth, eyeHeight);

        drawEye(&painter, leftEye);
        drawEye(&painter, rightEye);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton && event->type() == QEvent::MouseButtonDblClick) {
            close();
            event->accept();
            return;
        }

        if (!m_appearance.alwaysOnTop) {
            raise();
            activateWindow();
        }

        if (event->button() == Qt::LeftButton)
            m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();

        if (event->button() == Qt::RightButton) {
            showAppearanceDialog();
            event->accept();
            return;
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        if (event->buttons() & Qt::LeftButton)
            move(event->globalPosition().toPoint() - m_dragOffset);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            QApplication::quit();
            event->accept();
        }
    }

    void closeEvent(QCloseEvent *event) override
    {
        QWidget::closeEvent(event);
        QApplication::quit();
    }

private:
    class ColorPreview : public QWidget
    {
    public:
        explicit ColorPreview(QWidget *parent = nullptr)
            : QWidget(parent)
        {
            setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        }

        void setColor(const QColor &color)
        {
            m_color = color;
            update();
        }

        QColor color() const
        {
            return m_color;
        }

        void setClickHandler(std::function<void()> handler)
        {
            m_clickHandler = std::move(handler);
        }

        void setPreviewFont(const QFont &baseFont)
        {
            QFont previewFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            previewFont.setPointSizeF(baseFont.pointSizeF());
            previewFont.setPixelSize(baseFont.pixelSize());
            previewFont.setWeight(baseFont.weight());
            setFont(previewFont);
        }

        QSize sizeHint() const override
        {
            return QSize(96, 28);
        }

    protected:
        void paintEvent(QPaintEvent *) override
        {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing, true);

            const QRectF bounds = visualBounds();
            painter.setPen(QPen(m_color.darker(130), 1.0));
            painter.setBrush(m_color);
            painter.drawRoundedRect(bounds, 6.0, 6.0);

            painter.setPen(m_color.lightnessF() < 0.5 ? Qt::white : Qt::black);
            painter.setFont(font());
            painter.drawText(bounds.toRect(), Qt::AlignCenter, m_color.name(QColor::HexRgb));
        }

        void mousePressEvent(QMouseEvent *event) override
        {
            if (event->button() != Qt::LeftButton) {
                QWidget::mousePressEvent(event);
                return;
            }

            if (m_clickHandler)
                m_clickHandler();
            event->accept();
        }

    private:
        QRectF visualBounds() const
        {
#ifdef Q_OS_MACOS
            // On macOS, keep the full control height and only narrow the width slightly.
            return QRectF(rect()).adjusted(7.0, 0.5, -7.0, -0.5);
#else
            return QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
#endif
        }

        QColor m_color = Qt::white;
        std::function<void()> m_clickHandler;
    };

    struct EyeColors {
        QColor outline = Qt::black;
        QColor sclera = Qt::white;
        QColor iris = QColor(QStringLiteral("#3f7cac"));
        QColor pupil = Qt::black;
    };

    struct AppearanceSettings {
        EyeColors colors;
        bool alwaysOnTop = true;
    };

    qreal mmToPixelsX(qreal mm) const
    {
        const QScreen *currentScreen = screen();
        const qreal dpi = currentScreen ? currentScreen->logicalDotsPerInchX() : 96.0;
        return mm * dpi / 25.4;
    }

    qreal mmToPixelsY(qreal mm) const
    {
        const QScreen *currentScreen = screen();
        const qreal dpi = currentScreen ? currentScreen->logicalDotsPerInchY() : 96.0;
        return mm * dpi / 25.4;
    }

    void drawEye(QPainter *painter, const QRectF &eyeRect)
    {
        const qreal outlineWidth = 12.0;
        painter->setPen(QPen(m_appearance.colors.outline, outlineWidth));
        painter->setBrush(m_appearance.colors.sclera);
        painter->drawEllipse(eyeRect);

        const QPointF center = eyeRect.center();
        const QPoint globalCenter = mapToGlobal(center.toPoint());
        const QPoint cursor = QCursor::pos();

        const qreal dx = cursor.x() - globalCenter.x();
        const qreal dy = cursor.y() - globalCenter.y();

        const qreal radiusX = eyeRect.width() * 0.5;
        const qreal radiusY = eyeRect.height() * 0.5;
        const qreal irisWidth = eyeRect.width() * 0.32;
        const qreal irisHeight = eyeRect.height() * 0.34;
        const qreal pupilWidth = eyeRect.width() * 0.18;
        const qreal pupilHeight = eyeRect.height() * 0.20;
        const qreal minGapX = mmToPixelsX(5.0);
        const qreal minGapY = mmToPixelsY(5.0);
        const qreal innerMarginX = outlineWidth * 0.5 + minGapX;
        const qreal innerMarginY = outlineWidth * 0.5 + minGapY;
        const qreal movableWidth = qMax(irisWidth, pupilWidth);
        const qreal movableHeight = qMax(irisHeight, pupilHeight);
        const qreal baseLimitX = radiusX - movableWidth * 0.5 - innerMarginX;
        const qreal baseLimitY = radiusY - movableHeight * 0.5 - innerMarginY;
        const qreal targetLimitX = baseLimitX + mmToPixelsX(5.0);
        const qreal targetLimitY = baseLimitY + mmToPixelsY(8.0);
        const qreal limitX = qMax<qreal>(0.0, qMin(targetLimitX, radiusX - movableWidth * 0.5 - innerMarginX));
        const qreal limitY = qMax<qreal>(0.0, qMin(targetLimitY, radiusY - movableHeight * 0.5 - innerMarginY));

        QPointF pupilCenter = center;
        if ((!qFuzzyIsNull(dx) || !qFuzzyIsNull(dy)) && limitX > 0.0 && limitY > 0.0) {
            const qreal normalized = qSqrt((dx * dx) / (limitX * limitX) +
                                           (dy * dy) / (limitY * limitY));
            const qreal distanceScale = normalized > 1.0 ? 1.0 / normalized : 1.0;

            pupilCenter += QPointF(dx * distanceScale, dy * distanceScale);
        }

        painter->setPen(Qt::NoPen);
        painter->setBrush(m_appearance.colors.iris);
        painter->drawEllipse(QRectF(pupilCenter.x() - irisWidth * 0.5,
                                    pupilCenter.y() - irisHeight * 0.5,
                                    irisWidth,
                                    irisHeight));

        painter->setBrush(m_appearance.colors.pupil);
        painter->drawEllipse(QRectF(pupilCenter.x() - pupilWidth * 0.5,
                                    pupilCenter.y() - pupilHeight * 0.5,
                                    pupilWidth,
                                    pupilHeight));
    }

    static void updateColorPreview(ColorPreview *preview, const QColor &color)
    {
        preview->setColor(color);
    }

    void applyAlwaysOnTop()
    {
        const QRect geometryBeforeChange = geometry();
        const bool wasVisible = isVisible();

        setAttribute(Qt::WA_ShowWithoutActivating, m_appearance.alwaysOnTop);
        setAttribute(Qt::WA_MacAlwaysShowToolWindow, m_appearance.alwaysOnTop);
        if (m_appearance.alwaysOnTop) {
            setWindowFlags(Qt::FramelessWindowHint |
                           Qt::Tool |
                           Qt::WindowStaysOnTopHint |
                           Qt::NoDropShadowWindowHint);
        } else {
            setWindowFlags(Qt::FramelessWindowHint |
                           Qt::Window |
                           Qt::NoDropShadowWindowHint);
        }

        setGeometry(geometryBeforeChange);

        if (wasVisible) {
            show();
            raise();
            if (!m_appearance.alwaysOnTop)
                activateWindow();
        }
    }

    void loadSettings()
    {
        QSettings settings(QStringLiteral("qeyes"), QStringLiteral("qeyes"));
        m_appearance.colors.outline =
            settings.value(QStringLiteral("colors/outline"), m_appearance.colors.outline).value<QColor>();
        m_appearance.colors.sclera =
            settings.value(QStringLiteral("colors/sclera"), m_appearance.colors.sclera).value<QColor>();
        m_appearance.colors.iris =
            settings.value(QStringLiteral("colors/iris"), m_appearance.colors.iris).value<QColor>();
        m_appearance.colors.pupil =
            settings.value(QStringLiteral("colors/pupil"), m_appearance.colors.pupil).value<QColor>();
        m_appearance.alwaysOnTop =
            settings.value(QStringLiteral("window/alwaysOnTop"), m_appearance.alwaysOnTop).toBool();
    }

    void saveSettings() const
    {
        QSettings settings(QStringLiteral("qeyes"), QStringLiteral("qeyes"));
        settings.setValue(QStringLiteral("colors/outline"), m_appearance.colors.outline);
        settings.setValue(QStringLiteral("colors/sclera"), m_appearance.colors.sclera);
        settings.setValue(QStringLiteral("colors/iris"), m_appearance.colors.iris);
        settings.setValue(QStringLiteral("colors/pupil"), m_appearance.colors.pupil);
        settings.setValue(QStringLiteral("window/alwaysOnTop"), m_appearance.alwaysOnTop);
    }

    void showAppearanceDialog()
    {
        QDialog dialog(this);
        dialog.setWindowTitle(QStringLiteral("Appearance"));

        auto *layout = new QFormLayout(&dialog);

        AppearanceSettings pendingAppearance = m_appearance;

        const auto addColorRow = [&](const QString &label, QColor EyeColors::*member) {
            auto *rowWidget = new QWidget(&dialog);
            auto *rowLayout = new QHBoxLayout(rowWidget);
            rowLayout->setContentsMargins(0, 0, 0, 0);
            rowLayout->setAlignment(Qt::AlignVCenter);
            rowLayout->setSpacing(12);

            auto *labelWidget = new QLabel(label, rowWidget);

            auto *preview = new ColorPreview(rowWidget);
            auto *resetButton = new QPushButton(QStringLiteral("Reset"), rowWidget);
            preview->setFixedHeight(resetButton->sizeHint().height());
            preview->setPreviewFont(resetButton->font());
            labelWidget->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            labelWidget->setFixedWidth(72);
            updateColorPreview(preview, pendingAppearance.colors.*member);

            preview->setClickHandler([&dialog, &pendingAppearance, member, preview, this]() {
                const QColor color = QColorDialog::getColor(pendingAppearance.colors.*member, this,
                                                            QStringLiteral("Select Color"));
                if (!color.isValid())
                    return;

                pendingAppearance.colors.*member = color;
                updateColorPreview(preview, color);
            });

            connect(resetButton, &QPushButton::clicked, &dialog, [&pendingAppearance, member, preview, this]() {
                const EyeColors defaults;
                pendingAppearance.colors.*member = defaults.*member;
                updateColorPreview(preview, pendingAppearance.colors.*member);
            });

            rowLayout->addWidget(labelWidget);
            rowLayout->addWidget(preview);
            rowLayout->addWidget(resetButton);
            layout->addRow(rowWidget);
        };

        addColorRow(QStringLiteral("Outline"), &EyeColors::outline);
        addColorRow(QStringLiteral("Sclera"), &EyeColors::sclera);
        addColorRow(QStringLiteral("Iris"), &EyeColors::iris);
        addColorRow(QStringLiteral("Pupil"), &EyeColors::pupil);

        auto *alwaysOnTopCheck = new QCheckBox(QStringLiteral("Always on top"), &dialog);
        alwaysOnTopCheck->setChecked(pendingAppearance.alwaysOnTop);
        layout->addRow(alwaysOnTopCheck);

        auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addRow(buttonBox);

        if (dialog.exec() != QDialog::Accepted)
            return;

        pendingAppearance.alwaysOnTop = alwaysOnTopCheck->isChecked();
        m_appearance = pendingAppearance;
        applyAlwaysOnTop();
        saveSettings();
        update();
    }

    QPoint m_dragOffset;
    AppearanceSettings m_appearance;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(true);

    EyesWidget widget;
    widget.show();
    widget.raise();

    return app.exec();
}
