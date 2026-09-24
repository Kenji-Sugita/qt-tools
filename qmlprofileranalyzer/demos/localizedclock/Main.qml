import QtQuick
import QtQuick.Layouts

Window {
    id: root
    width: 320
    height: 320
    visible: true
    minimumWidth: layout.Layout.minimumWidth
    minimumHeight: layout.Layout.minimumHeight

    title: qsTr("Digital Clock")

    property string time
    property string date
    property int seconds

    ColumnLayout {
        id: layout
        anchors.fill: parent

        Text {
            text: root.time
            fontSizeMode: Text.Fit
            font.pixelSize: 60
            minimumPixelSize: 10
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("%n second(s)", "seconds", root.seconds)
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: root.date
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: qsTr("Locale: %1").arg(Qt.locale().name)
            font.pixelSize: 20
            Layout.alignment: Qt.AlignHCenter
        }
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true

        onTriggered: {
            const now = new Date();
            const locale = Qt.locale();
            root.time = now.toLocaleTimeString(locale, Locale.ShortFormat);
            root.date = now.toLocaleDateString(locale);
            root.seconds = now.getSeconds();
        }
    }
}
