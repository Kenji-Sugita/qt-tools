#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QEasingCurve>
#include <QColor>

class AnimatedCircleWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QPoint circleCenter READ circleCenter WRITE setCircleCenter)
    Q_PROPERTY(int circleDiameter READ circleDiameter WRITE setCircleDiameter)
    Q_PROPERTY(QColor circleColor READ circleColor WRITE setCircleColor)

public:
    explicit AnimatedCircleWidget(QWidget* parent = nullptr)
        : QWidget(parent)
        , currentCircleCenter(80, 100)
        , currentCircleDiameter(60)
        , currentCircleColor(52, 152, 219)
    {
        setMinimumSize(500, 220);
        setAutoFillBackground(true);
    }

    QPoint circleCenter() const
    {
        return currentCircleCenter;
    }

    void setCircleCenter(const QPoint& newCircleCenter)
    {
        if (currentCircleCenter == newCircleCenter) {
            return;
        }
        currentCircleCenter = newCircleCenter;
        update();
    }

    int circleDiameter() const
    {
        return currentCircleDiameter;
    }

    void setCircleDiameter(int newCircleDiameter)
    {
        if (currentCircleDiameter == newCircleDiameter) {
            return;
        }
        currentCircleDiameter = newCircleDiameter;
        update();
    }

    QColor circleColor() const
    {
        return currentCircleColor;
    }

    void setCircleColor(const QColor& newCircleColor)
    {
        if (currentCircleColor == newCircleColor) {
            return;
        }
        currentCircleColor = newCircleColor;
        update();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.fillRect(rect(), palette().window());

        painter.setPen(Qt::NoPen);
        painter.setBrush(currentCircleColor);

        const int radius = currentCircleDiameter / 2;
        const QRect circleRectangle(
            currentCircleCenter.x() - radius,
            currentCircleCenter.y() - radius,
            currentCircleDiameter,
            currentCircleDiameter);

        painter.drawEllipse(circleRectangle);
    }

private:
    QPoint currentCircleCenter;
    int currentCircleDiameter;
    QColor currentCircleColor;
};

class AnimationDemoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnimationDemoWidget(QWidget* parent = nullptr)
        : QWidget(parent)
        , animationView(new AnimatedCircleWidget(this))
        , startButton(new QPushButton("Start Animation", this))
        , descriptionLabel(new QLabel("We perform movement, scaling, and color changes simultaneously.", this))
        , animationGroup(new QSequentialAnimationGroup(this))
    {
        setWindowTitle(QStringLiteral("Qt Widgets Animation"));

        descriptionLabel->setAlignment(Qt::AlignCenter);

        auto* buttonLayout = new QHBoxLayout;
        buttonLayout->addStretch();
        buttonLayout->addWidget(startButton);
        buttonLayout->addStretch();

        auto* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(descriptionLabel);
        mainLayout->addWidget(animationView, 1);
        mainLayout->addLayout(buttonLayout);

        createAnimation();

        connect(startButton, &QPushButton::clicked,
                this, &AnimationDemoWidget::startAnimation);
    }

private:
    void createAnimation()
    {
        auto* moveForwardAnimation =
            new QPropertyAnimation(animationView, "circleCenter", this);
        moveForwardAnimation->setDuration(1200);
        moveForwardAnimation->setStartValue(QPoint(80, 100));
        moveForwardAnimation->setEndValue(QPoint(420, 100));
        moveForwardAnimation->setEasingCurve(QEasingCurve::InOutCubic);

        auto* growAnimation =
            new QPropertyAnimation(animationView, "circleDiameter", this);
        growAnimation->setDuration(1200);
        growAnimation->setStartValue(60);
        growAnimation->setEndValue(120);
        growAnimation->setEasingCurve(QEasingCurve::OutBack);

        auto* changeToRedAnimation =
            new QPropertyAnimation(animationView, "circleColor", this);
        changeToRedAnimation->setDuration(1200);
        changeToRedAnimation->setStartValue(QColor(52, 152, 219));
        changeToRedAnimation->setEndValue(QColor(231, 76, 60));
        changeToRedAnimation->setEasingCurve(QEasingCurve::Linear);

        auto* forwardAnimationGroup = new QParallelAnimationGroup(this);
        forwardAnimationGroup->addAnimation(moveForwardAnimation);
        forwardAnimationGroup->addAnimation(growAnimation);
        forwardAnimationGroup->addAnimation(changeToRedAnimation);

        auto* moveBackwardAnimation =
            new QPropertyAnimation(animationView, "circleCenter", this);
        moveBackwardAnimation->setDuration(1000);
        moveBackwardAnimation->setStartValue(QPoint(420, 100));
        moveBackwardAnimation->setEndValue(QPoint(80, 100));
        moveBackwardAnimation->setEasingCurve(QEasingCurve::InOutQuad);

        auto* shrinkAnimation =
            new QPropertyAnimation(animationView, "circleDiameter", this);
        shrinkAnimation->setDuration(1000);
        shrinkAnimation->setStartValue(120);
        shrinkAnimation->setEndValue(60);
        shrinkAnimation->setEasingCurve(QEasingCurve::InBack);

        auto* changeToBlueAnimation =
            new QPropertyAnimation(animationView, "circleColor", this);
        changeToBlueAnimation->setDuration(1000);
        changeToBlueAnimation->setStartValue(QColor(231, 76, 60));
        changeToBlueAnimation->setEndValue(QColor(52, 152, 219));
        changeToBlueAnimation->setEasingCurve(QEasingCurve::Linear);

        auto* backwardAnimationGroup = new QParallelAnimationGroup(this);
        backwardAnimationGroup->addAnimation(moveBackwardAnimation);
        backwardAnimationGroup->addAnimation(shrinkAnimation);
        backwardAnimationGroup->addAnimation(changeToBlueAnimation);

        animationGroup->addAnimation(forwardAnimationGroup);
        animationGroup->addPause(250);
        animationGroup->addAnimation(backwardAnimationGroup);
    }

    void startAnimation()
    {
        if (animationGroup->state() == QAbstractAnimation::Running) {
            return;
        }

        animationGroup->start();
    }

private:
    AnimatedCircleWidget* animationView;
    QPushButton* startButton;
    QLabel* descriptionLabel;
    QSequentialAnimationGroup* animationGroup;
};

AnimationDemoWidget* create_animation()
{
    auto demoWidget = new AnimationDemoWidget{};
    demoWidget->resize(560, 320);
    demoWidget->show();
    demoWidget->raise();
    demoWidget->activateWindow();

    return demoWidget;
}

auto animation = create_animation();

#include "animation.moc"
