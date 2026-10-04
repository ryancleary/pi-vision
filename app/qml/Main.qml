import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import PiVision
import PiVision.Display

ApplicationWindow {
    id: root

    width: 1100
    height: 640
    visible: true
    title: "pi-vision"
    color: Theme.background

    // Dark theme for every Basic-style control.
    palette.window: Theme.panel
    palette.windowText: Theme.text
    palette.base: Theme.background
    palette.text: Theme.text
    palette.button: Theme.control
    palette.buttonText: Theme.text
    palette.highlight: Theme.accent
    palette.highlightedText: Theme.background
    palette.mid: Theme.control
    palette.midlight: Theme.control // switch track when off, slider groove
    palette.dark: Theme.accent      // switch track when on, slider fill
    palette.light: Theme.control
    palette.placeholderText: Theme.muted

    // Top: the status line (only when ui.json reserves a top inset) and the
    // toolbar if it sits at the top. Bottom: the toolbar if it sits there.
    header: Column {
        StatusLine {
            width: parent.width
            height: UiConfig.topInset
            visible: UiConfig.topInset > 0
        }

        Item {
            id: topToolbarSlot

            width: parent.width
            height: UiConfig.toolbarAtBottom ? 0 : toolbar.implicitHeight
        }
    }

    footer: Item {
        id: bottomToolbarSlot

        height: UiConfig.toolbarAtBottom ? toolbar.implicitHeight : 0
    }

    ToolBar {
        id: toolbar

        // Lives in whichever slot ui.json picks ("toolbar": "top" or "bottom").
        parent: UiConfig.toolbarAtBottom ? bottomToolbarSlot : topToolbarSlot
        width: parent.width
        background: Rectangle { color: Theme.panel }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.margin
            anchors.rightMargin: Theme.margin
            spacing: Theme.spacing

            Label {
                text: qsTr("Source")
                color: Theme.muted
            }

            // The source box and the opacity slider give up width first when
            // the toolbar is tight (1024 px screens); the buttons keep theirs.
            ComboBox {
                id: sourceBox

                Layout.fillWidth: true
                Layout.minimumWidth: Theme.sourceBoxMinWidth
                Layout.maximumWidth: Theme.sourceBoxWidth
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

            Item { Layout.fillWidth: true; Layout.minimumWidth: 0 }

            // How strongly the processed image covers the raw one in overlay mode.
            Label {
                visible: compareView.mode === CompareView.ViewMode.Overlay
                text: qsTr("Processed %1%").arg(Math.round(overlayMix.value * 100))
                color: Theme.muted
            }

            Slider {
                id: overlayMix

                Layout.fillWidth: true
                Layout.minimumWidth: Theme.sliderMinWidth
                Layout.maximumWidth: Theme.sliderWidth
                visible: compareView.mode === CompareView.ViewMode.Overlay
                from: 0
                to: 1
                value: 0.5
            }

            RowLayout {
                spacing: Theme.buttonGap

                Repeater {
                    // Order and choice of buttons come from config/ui.json.
                    model: UiConfig.modeButtons.map(name => compareView.modeFromName(name))

                    delegate: PanelButton {
                        required property int modelData

                        label: compareView.modeLabel(modelData)
                        active: compareView.mode === modelData
                        onClicked: compareView.mode = modelData
                    }
                }
            }

            // Saves both frames, the settings and the metrics (see Captures.directory).
            PanelButton {
                Layout.leftMargin: Theme.spacing
                label: Captures.busy ? qsTr("Saving…") : qsTr("Capture")
                enabled: !Captures.busy
                onClicked: Captures.take()
            }

            // Only while a stick is in: copies captures and crash dumps onto it.
            PanelButton {
                visible: Usb.present
                label: Usb.busy ? qsTr("Copying…") : qsTr("Copy to USB")
                enabled: !Usb.busy
                onClicked: Usb.copyAll()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Detection runs only while Detection mode is shown.
            Binding {
                target: Detection
                property: "enabled"
                value: compareView.mode === CompareView.ViewMode.Detection
            }

            CompareView {
                id: compareView

                anchors.fill: parent
                anchors.margins: Theme.margin
                overlayOpacity: overlayMix.value
                // Starting mode from config/ui.json; the buttons change it after.
                Component.onCompleted: mode = modeFromName(UiConfig.defaultMode)
            }

            // A scenario (system test) switches modes the way the buttons do.
            Connections {
                target: Scenario
                function onModeRequested(mode) { compareView.mode = compareView.modeFromName(mode) }
            }

            // Short messages (capture and USB results), shown for a few seconds.
            Label {
                id: toast

                property bool isError: false

                function show(message, error) {
                    text = message
                    isError = error
                    opacity = 1
                    toastTimer.restart()
                }

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Theme.margin * 2
                z: 10 // over the compare view
                padding: Theme.spacing
                opacity: 0
                color: isError ? Theme.error : Theme.text
                font.pixelSize: Theme.fontSize
                background: Rectangle {
                    color: Theme.panel
                    border.color: Theme.stroke
                    radius: Theme.radius
                }

                Behavior on opacity { NumberAnimation { duration: Theme.transitionMs } }

                Timer {
                    id: toastTimer

                    interval: Theme.toastMs
                    onTriggered: toast.opacity = 0
                }

                Connections {
                    target: Captures
                    function onSaved(name) { toast.show(qsTr("Saved capture %1").arg(name), false) }
                    function onFailed(message) { toast.show(message, true) }
                }

                Connections {
                    target: Usb
                    function onCopied(summary) { toast.show(summary, false) }
                    function onFailed(message) { toast.show(message, true) }
                    function onPresentChanged() {
                        if (Usb.present)
                            toast.show(qsTr("USB stick found at %1").arg(Usb.mountPoint), false)
                    }
                }
            }

            // Shown when the source fails; the feed is cut, so this is all that's visible.
            Label {
                anchors.centerIn: parent
                width: parent.width * Theme.messageWidthFraction
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                visible: Pipeline.errorString !== ""
                text: Pipeline.errorString
                color: Theme.error
                font.pixelSize: Theme.messageSize
            }
        }

        // Side panel: processing stages, or live metrics.
        Rectangle {
            Layout.preferredWidth: Theme.sidePanelWidth
            Layout.fillHeight: true
            color: Theme.panel

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                TabBar {
                    id: panelTabs

                    Layout.fillWidth: true
                    background: Rectangle { color: Theme.background }

                    PanelTab { text: qsTr("Stages") }
                    PanelTab { text: qsTr("Metrics") }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: Theme.margin
                    currentIndex: panelTabs.currentIndex

                    ScrollView {
                        contentWidth: availableWidth

                        ColumnLayout {
                            id: stageList

                            width: parent.width
                            spacing: 10
                        }
                    }

                    MetricsPanel { }
                }
            }
        }
    }

    // A side-panel tab: panel-colored with an accent underline when selected.
    // (The Basic style would fill unselected tabs with palette.dark, our accent.)
    component PanelTab: TabButton {
        id: tab

        implicitHeight: Theme.tabHeight
        font.pixelSize: Theme.headingSize

        contentItem: Text {
            text: tab.text
            font: tab.font
            color: tab.checked ? Theme.text : Theme.muted
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            color: tab.checked ? Theme.panel : Theme.background

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: Theme.tabIndicatorHeight
                visible: tab.checked
                color: Theme.accent
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
            spacing: Theme.spacing

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
