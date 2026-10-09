import QtQuick
import PiVision

// A flat toolbar button. "active" draws it in the accent color, for the
// selected entry of a group such as the compare modes.
Rectangle {
    id: control

    property string label
    property bool active: false

    signal clicked()

    readonly property color restingColor: control.active ? Theme.accent : Theme.control

    implicitWidth: Theme.buttonWidth
    implicitHeight: Theme.buttonHeight

    radius: Theme.radius
    border.color: Theme.stroke
    border.width: Theme.borderWidth
    opacity: control.enabled ? 1.0 : 0.5
    color: pressArea.pressed ? Qt.darker(control.restingColor, 1.15) : control.restingColor

    Text {
        anchors.centerIn: parent
        text: control.label
        // Dark text on the accent fill, light text otherwise.
        color: control.active ? Theme.background : Theme.text
        font.pixelSize: Theme.fontSize
    }

    MouseArea {
        id: pressArea

        anchors.fill: parent
        onClicked: control.clicked()
    }
}
