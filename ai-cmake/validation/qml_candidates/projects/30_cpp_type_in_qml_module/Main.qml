import QtQuick
import Qml30CppTypeModule

Item {
    width: 320
    height: 240
    property var counter: Counter {}
    property int answer: counter.value
}
