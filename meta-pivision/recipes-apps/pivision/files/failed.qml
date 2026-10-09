import QtQuick

Window {
    id: root
    visible: true
    visibility: Window.FullScreen
    color: "#1b1f24"

    property string log: ""

    Component.onCompleted: {
        const request = new XMLHttpRequest();
        request.open("GET", "file:///run/pivision-failed.log", false);
        request.send();
        log = request.responseText;
    }

    Column {
        anchors.centerIn: parent
        width: parent.width * 0.85
        spacing: 16

        Text {
            text: "pi-vision failed to start"
            color: "#ff8a80"
            font { pixelSize: 36; bold: true }
        }
        Text {
            width: parent.width
            wrapMode: Text.Wrap
            color: "#e6e6e6"
            font.pixelSize: 20
            text: "Connect over SSH as root and run: journalctl -u pivision"
        }
        Text {
            width: parent.width
            wrapMode: Text.WrapAnywhere
            color: "#9aa4ad"
            font { pixelSize: 14; family: "monospace" }
            text: root.log
        }
    }
}

