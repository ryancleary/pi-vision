import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import PiVision.Display

ApplicationWindow {
    id: root

    width: 1100
    height: 640
    visible: true
    title: "pi-vision"
    color: colors.background

    QtObject {
        id: colors
        readonly property color background: "#1b1f24"
        readonly property color panel: "#262b31"
        readonly property color control: "#343a42"
        readonly property color text: "#e6e6e6"
        readonly property color muted: "#9aa4ad"
        readonly property color accent: "#3cc8ff"
        readonly property color error: "#ff8a80"
    }

    // Dark theme for every Basic-style control.
    palette.window: colors.panel
    palette.windowText: colors.text
    palette.base: colors.background
    palette.text: colors.text
    palette.button: colors.control
    palette.buttonText: colors.text
    palette.highlight: colors.accent
    palette.highlightedText: colors.background
    palette.mid: colors.control
    palette.midlight: colors.control // switch track when off, slider groove
    palette.dark: colors.accent      // switch track when on, slider fill
    palette.light: colors.control
    palette.placeholderText: colors.muted

    header: ToolBar {
        background: Rectangle { color: colors.panel }

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

            // Temporary until the compare view (side by side, overlay, picture-in-picture).
            Switch {
                id: rawSwitch
                text: qsTr("Show raw")
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            FrameView {
                anchors.fill: parent
                pipeline: Pipeline
                stream: rawSwitch.checked ? FrameView.Raw : FrameView.Processed
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
        }

        Rectangle {
            Layout.preferredWidth: 280
            Layout.fillHeight: true
            color: colors.panel

            ScrollView {
                anchors.fill: parent
                anchors.margins: 12
                contentWidth: availableWidth

                ColumnLayout {
                    id: stageList

                    width: parent.width
                    spacing: 10

                    Label {
                        text: qsTr("Processing")
                        color: colors.muted
                        font.pixelSize: 13
                    }
                }
            }
        }
    }

    // One control per stage, created from the stages.json entries at startup.
    // Done with createComponent/createObject rather than a Repeater so the panel
    // is built entirely from data at runtime.
    Component.onCompleted: {
        const component = Qt.createComponent("StageControl.qml")
        if (component.status !== Component.Ready) {
            console.error("StageControl.qml:", component.errorString())
            return
        }
        for (const stage of ProcessingControl.stages) {
            const control = component.createObject(stageList, {
                stageId: stage.id,
                label: stage.label,
                active: stage.enabled,
                parameters: stage.parameters
            })
            if (control === null)
                console.error("Could not create the control for stage", stage.id)
        }
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
