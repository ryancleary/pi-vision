import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import PiVision
import PiVision.Display

// Information only, no controls: fills the top inset that ui.json reserves on
// screens whose touch panel can't sense that strip (see UiConfig.h).
Rectangle {
    id: line

    color: Theme.background

    // The detector's state as words, shown while Detection mode is on.
    function detectorText() {
        switch (Detection.state) {
        case Detection.Loading: return qsTr("Detector loading…")
        case Detection.Ready: return qsTr("Detector ready")
        case Detection.Failed: return qsTr("Detector failed")
        default: return ""
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.margin
        anchors.rightMargin: Theme.margin
        spacing: Theme.spacing

        Label {
            text: "pi-vision"
            color: Theme.text
            font.pixelSize: Theme.headingSize
        }

        Label {
            Layout.fillWidth: true
            text: Pipeline.sourceName
            color: Theme.muted
            elide: Text.ElideRight
            font.pixelSize: Theme.fontSize
        }

        Label {
            visible: Detection.enabled
            text: line.detectorText()
            color: Detection.state === Detection.Failed ? Theme.error : Theme.muted
            font.pixelSize: Theme.fontSize
        }
    }
}
