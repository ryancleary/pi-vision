import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import PiVision.Display

ApplicationWindow {
    id: root

    width: 960
    height: 600
    visible: true
    title: "pi-vision"
    color: colors.background

    QtObject {
        id: colors
        readonly property color background: "#1b1f24"
        readonly property color bar: "#262b31"
        readonly property color text: "#e6e6e6"
        readonly property color muted: "#9aa4ad"
        readonly property color error: "#ff8a80"
    }

    header: ToolBar {
        background: Rectangle { color: colors.bar }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            spacing: 8

            Label {
                text: qsTr("Source")
                color: colors.muted
            }

            ComboBox {
                id: sourceBox

                Layout.preferredWidth: 340
                textRole: "label"
                // Cameras and the test pattern, then "Other..." for anything typed in.
                model: SourceSelector.sources.concat([{ label: qsTr("Other…"), spec: "" }])
                // Always show what is actually running, not the last item clicked.
                displayText: Pipeline.sourceName

                onActivated: (index) => {
                    const choice = model[index]
                    if (choice.spec === "")
                        otherDialog.open()
                    else
                        SourceSelector.select(choice.spec)
                }

                // Rescan each time the list opens, so a camera plugged in later shows up.
                Connections {
                    target: sourceBox.popup
                    function onAboutToShow() { SourceSelector.refresh() }
                }
            }

            Item { Layout.fillWidth: true }
        }
    }

    FrameView {
        anchors.fill: parent
        pipeline: Pipeline
    }

    // Shown when the source fails; the feed is cut, so this is all that's visible.
    Label {
        anchors.centerIn: parent
        width: parent.width * 0.8
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
        visible: Pipeline.errorString !== ""
        text: Pipeline.errorString
        color: colors.error
        font.pixelSize: 18
    }

    Dialog {
        id: otherDialog

        // Centered in the window's content area, below the toolbar.
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        modal: true
        title: qsTr("Open a source")
        standardButtons: Dialog.Open | Dialog.Cancel

        onOpened: {
            specField.text = ""
            specField.forceActiveFocus()
        }
        onAccepted: SourceSelector.select(specField.text)

        ColumnLayout {
            spacing: 8

            Label { text: qsTr("Camera device, video file or stream URL") }

            TextField {
                id: specField

                Layout.preferredWidth: 380
                placeholderText: "/dev/video2, ~/clip.mp4, rtsp://…"
                onAccepted: otherDialog.accept()
            }
        }
    }
}
