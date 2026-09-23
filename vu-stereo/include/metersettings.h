// MeterSettings は、VU メーターの表示と入力補正に関する設定値をまとめる
// 軽量なデータ構造です。設定ダイアログ、表示パネル、音声レベル制御の間で
// 同じ値を受け渡すために使い、各クラスが個別に設定項目を持って重複することを
// 避けます。

#pragma once

#include "translationmanager.h"

enum class MeterScaleStyle {
    Arc,
    Linear,
};

struct MeterSettings {
    int needleWidth = 6;
    int inputLevelDb = 0;
    bool pivotVisible = true;
    bool needlePivotLowered = true;
    bool peakHoldEnabled = false;
    bool alwaysOnTop = false;
    MeterScaleStyle scaleStyle = MeterScaleStyle::Arc;
    AppLanguage language = AppLanguage::System;
};
