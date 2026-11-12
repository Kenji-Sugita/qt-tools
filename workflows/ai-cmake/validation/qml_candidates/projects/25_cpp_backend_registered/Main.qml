import QtQuick
import Qml25CppBackend

Item {
    width: 320
    height: 240
    property var backend: Backend {}
    property string text: backend.message
}
