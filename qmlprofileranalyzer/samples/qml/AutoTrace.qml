import QtQuick
import QtQuick.Window

Window {
    id: root
    width: 240
    height: 180
    visible: true
    title: "qmlprofileranalyzer demo"

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#f6f1e8"
    }

    Rectangle {
        id: box
        width: 48
        height: 48
        radius: 8
        y: 66
        color: "#0f766e"
        x: 20

        NumberAnimation on x {
            from: 20
            to: 160
            duration: 180
            loops: Animation.Infinite
            easing.type: Easing.InOutQuad
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        text: "trace demo"
        color: "#1f2937"
    }

    Timer {
        interval: 1200
        running: true
        repeat: false
        onTriggered: Qt.quit()
    }
}
