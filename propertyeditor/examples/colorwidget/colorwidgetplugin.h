#pragma once

#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

class ColorWidgetPlugin
    : public QObject
    , public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface"
                      FILE "colorwidget.json")
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit ColorWidgetPlugin(QObject* parent = nullptr);

    // QDesignerCustomWidgetInterface
    QWidget* createWidget(QWidget* parent) override;
    QString name() const override;
    QString group() const override;
    QIcon icon() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    bool isContainer() const override;
    QString includeFile() const override;
    QString domXml() const override;

    void initialize(QDesignerFormEditorInterface* core) override;
    bool isInitialized() const override;

private:
    bool m_initialized = false;
};
