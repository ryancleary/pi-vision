import QtQuick
import QtQuick.Controls.Basic
import PiVision.Display

ApplicationWindow {
    width: 960
    height: 600
    visible: true
    title: "pi-vision"
    color: "#1b1f24"

    FrameView {
        anchors.fill: parent
        pipeline: Pipeline
    }

    Label {
        id: status
        anchors { left: parent.left; bottom: parent.bottom; margins: 12 }
        color: "#e6e6e6"
        text: Pipeline.sourceName
    }

    Connections {
        target: Pipeline
        function onFailed(message) { status.text = message }
    }
}

