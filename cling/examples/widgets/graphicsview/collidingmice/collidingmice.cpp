// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtMath>
#include <QtWidgets>

#include "mouse.h"

static int MouseCount = 7;

//! [0]
QGraphicsView* collidingmice()
{
//! [0]

//! [1]
    auto scene = new QGraphicsScene{};
    scene->setSceneRect(-300, -300, 600, 600);
//! [1] //! [2]
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
//! [2]

//! [3]
    for (int i = 0; i < MouseCount; ++i) {
        Mouse *mouse = new Mouse;
        mouse->setPos(::sin((i * 6.28) / MouseCount) * 200,
                      ::cos((i * 6.28) / MouseCount) * 200);
        scene->addItem(mouse);
    }
//! [3]

//! [4]
    auto view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QPixmap(":/images/cheese.jpg"));
//! [4] //! [5]
    view->setCacheMode(QGraphicsView::CacheBackground);
    view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    view->setDragMode(QGraphicsView::ScrollHandDrag);
//! [5] //! [6]
    view->setWindowTitle(QT_TRANSLATE_NOOP(QGraphicsView, "Colliding Mice"));
    view->resize(400, 300);
    view->show();

    auto timer = new QTimer{};
    QObject::connect(timer, &QTimer::timeout, scene, &QGraphicsScene::advance);
    timer->start(1000 / 33);

    view->show();
    view->raise();
    view->activateWindow();

    return view;
}
//! [6]

auto view = collidingmice();

void addMouse(int n = 1)
{
    for (int i = 0; i < n; ++i) {
        qDebug() << i;
        Mouse *mouse = new Mouse;
        mouse->setPos(::sin((i * 6.28) / MouseCount) * 200,
                    ::cos((i * 6.28) / MouseCount) * 200);
        view->scene()->addItem(mouse);
    }

    MouseCount += n;
}
