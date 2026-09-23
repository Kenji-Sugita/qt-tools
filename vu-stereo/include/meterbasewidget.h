// MeterBaseWidget は、左右の VU メーターを載せる暗い台座を描画する
// ウィジェットです。メーター本体や音声値は扱わず、アプリケーションの外観を構成する
// 背景パーツとして、透明なフレームレスウィンドウ上に台座だけを描きます。

#pragma once

#include <QWidget>

class MeterBaseWidget final : public QWidget {
public:
    explicit MeterBaseWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
};
