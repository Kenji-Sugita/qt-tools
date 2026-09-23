#include "colorwidgetplugin.h"
#include "colorwidget.h"

ColorWidgetPlugin::ColorWidgetPlugin(QObject* parent)
    : QObject{parent}
{
}

void ColorWidgetPlugin::initialize(QDesignerFormEditorInterface* /* core */)
{
    if (m_initialized) {
        return;
    }
    m_initialized = true;
}

bool ColorWidgetPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget* ColorWidgetPlugin::createWidget(QWidget* parent)
{
    return new ColorWidget{parent};
}

QString ColorWidgetPlugin::name() const
{
    return QStringLiteral("ColorWidget");
}

QString ColorWidgetPlugin::group() const
{
    return QStringLiteral("Custom Widgets [Example]");
}

QIcon ColorWidgetPlugin::icon() const
{
    return {};
}

QString ColorWidgetPlugin::toolTip() const
{
    return QStringLiteral("RGB スライダで色を選択するウィジェット");
}

QString ColorWidgetPlugin::whatsThis() const
{
    return QStringLiteral("RGB スライダとプレビューを持つ色選択ウィジェットです。");
}

bool ColorWidgetPlugin::isContainer() const
{
    return false;
}

QString ColorWidgetPlugin::includeFile() const
{
    return QStringLiteral("colorwidget.h");
}

QString ColorWidgetPlugin::domXml() const
{
    return QStringLiteral(R"(
<ui language="c++">
 <widget class="ColorWidget" name="colorWidget">
  <property name="toolTip">
   <string>RGB カラーピッカー</string>
  </property>
  <property name="whatsThis">
   <string>RGB スライダで色を設定するカスタムウィジェット</string>
  </property>
 </widget>
</ui>
)");
}
