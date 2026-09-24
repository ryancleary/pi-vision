import QtQuick
import QtQuick.Controls.Basic
import PiVision
import PiVision.Display

// The raw and processed images of the pipeline, compared three ways, plus
// object detection:
//   SideBySide        raw on the left, processed on the right
//   Overlay           processed drawn over raw, faded by overlayOpacity
//   PictureInPicture  processed fills the view, raw inset in the bottom-right corner
//   Detection         one image with the detector's boxes: the live raw or
//                     processed video, or (showDetectedFrame) the exact frame
//                     the boxes were found on, inside the model's input
// Switching modes animates the panes from one layout to the other.
//
// Overlay lines up because the processed image is a downscale of the raw one
// with the same aspect ratio, and both are fitted to the same pane.
Item {
    id: view

    enum ViewMode {
        SideBySide,
        Overlay,
        PictureInPicture,
        Detection
    }

    property int mode: CompareView.ViewMode.Overlay
    // Detection mode: false shows live video with the newest boxes, which
    // trail moving objects by the detection time; true shows the frame the
    // boxes were found on, so they line up exactly but update less often.
    property bool showDetectedFrame: false

    // The mode for a name used in config/ui.json (see knownViewModes() in
    // UiConfig.cpp, which checks the file against the same names).
    function modeFromName(name) {
        switch (name) {
        case "sideBySide": return CompareView.ViewMode.SideBySide
        case "overlay": return CompareView.ViewMode.Overlay
        case "pictureInPicture": return CompareView.ViewMode.PictureInPicture
        case "detection": return CompareView.ViewMode.Detection
        }
        console.warn("Unknown view mode", name)
        return CompareView.ViewMode.Overlay
    }

    // Button text for each mode.
    function modeLabel(viewMode) {
        switch (viewMode) {
        case CompareView.ViewMode.SideBySide: return qsTr("Side by side")
        case CompareView.ViewMode.Overlay: return qsTr("Overlay")
        case CompareView.ViewMode.PictureInPicture: return qsTr("PiP")
        case CompareView.ViewMode.Detection: return qsTr("Detection")
        }
        return ""
    }
    property real overlayOpacity: 0.5

    // Side by side: two equal panes with a gap between them.
    readonly property real halfWidth: (view.width - Theme.paneGap) / 2

    // One stream in a framed pane, with a caption in the top-left corner.
    component StreamPane: Rectangle {
        id: pane

        property alias stream: frame.stream
        property string caption
        property bool captioned: true

        color: Theme.background
        border.color: Theme.stroke
        border.width: Theme.borderWidth
        radius: Theme.radius
        clip: true
        // A fully faded pane isn't drawn at all, which saves the Pi uploading its frames.
        visible: opacity > 0

        FrameView {
            id: frame

            anchors.fill: parent
            anchors.margins: Theme.borderWidth
            pipeline: Pipeline
        }

        // On a translucent chip so it stays readable over a bright image.
        Label {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: Theme.spacing
            visible: pane.captioned
            padding: Theme.captionPadding
            text: pane.caption
            color: Theme.text
            font.pixelSize: Theme.fontSize
            background: Rectangle {
                color: Theme.background
                opacity: Theme.captionOpacity
                radius: Theme.radius
            }
        }
    }

    // Detection mode's single pane: an image with the detector's boxes on it.
    component DetectionPane: Rectangle {
        id: pane

        color: Theme.background
        border.color: Theme.stroke
        border.width: Theme.borderWidth
        radius: Theme.radius
        clip: true
        visible: opacity > 0

        // Detected-frame view only: the model's input area, with the parts the
        // image doesn't cover (the letterbox padding) in gray, so you see what
        // the model looked at. It's the largest rectangle with the model's
        // aspect ratio that fits the pane.
        Rectangle {
            id: modelInput

            readonly property real availableWidth: pane.width - 2 * Theme.borderWidth
            readonly property real availableHeight: pane.height - 2 * Theme.borderWidth

            visible: view.showDetectedFrame && Detection.modelAspect > 0
            anchors.centerIn: parent
            width: Math.min(availableWidth, availableHeight * Detection.modelAspect)
            height: visible ? width / Detection.modelAspect : 0
            color: Theme.modelPadding
        }

        FrameView {
            id: frame

            // Fitted into the model's input in the detected-frame view, so the
            // image sits in it the way the model saw it; else the whole pane.
            x: modelInput.visible ? modelInput.x : Theme.borderWidth
            y: modelInput.visible ? modelInput.y : Theme.borderWidth
            width: modelInput.visible ? modelInput.width : modelInput.availableWidth
            height: modelInput.visible ? modelInput.height : modelInput.availableHeight
            pipeline: Pipeline
            stream: view.showDetectedFrame ? FrameView.Detected
                  : Detection.input === Detection.Processed ? FrameView.Processed
                  : FrameView.Raw
        }

        // One box per detection. Boxes come as fractions of the image, so
        // they're placed relative to where the image is drawn (imageRect).
        Repeater {
            model: Detection.boxes

            delegate: Rectangle {
                required property var modelData

                x: frame.x + frame.imageRect.x + modelData.x * frame.imageRect.width
                y: frame.y + frame.imageRect.y + modelData.y * frame.imageRect.height
                width: modelData.width * frame.imageRect.width
                height: modelData.height * frame.imageRect.height
                color: "transparent"
                border.color: Theme.detectionBox
                border.width: Theme.detectionBoxWidth

                // "person 76%", on the box's top edge.
                Label {
                    anchors.left: parent.left
                    anchors.bottom: parent.top
                    padding: Theme.captionPadding
                    text: qsTr("%1 %2%").arg(parent.modelData.label).arg(Math.round(parent.modelData.score * 100))
                    color: Theme.detectionLabelText
                    font.pixelSize: Theme.fontSize
                    background: Rectangle { color: Theme.detectionBox }
                }
            }
        }

        // What's shown, and the detector's state when it isn't running yet.
        Label {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.margins: Theme.spacing
            padding: Theme.captionPadding
            text: (view.showDetectedFrame ? qsTr("Detected frame") : qsTr("Live"))
                  + " · " + (Detection.input === Detection.Processed ? qsTr("processed") : qsTr("raw"))
            color: Theme.text
            font.pixelSize: Theme.fontSize
            background: Rectangle {
                color: Theme.background
                opacity: Theme.captionOpacity
                radius: Theme.radius
            }
        }

        // What the detector looks at, and what's shown; along the bottom edge,
        // since the toolbar is full.
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Theme.margin
            width: controls.implicitWidth + 2 * Theme.spacing
            height: controls.implicitHeight + 2 * Theme.spacing
            radius: Theme.radius
            // Translucent background; opacity would fade the buttons too.
            color: Qt.rgba(Theme.panel.r, Theme.panel.g, Theme.panel.b, Theme.captionOpacity)

            Row {
                id: controls

                anchors.centerIn: parent
                spacing: Theme.buttonGap

                PanelButton {
                    label: qsTr("Raw")
                    active: Detection.input === Detection.Raw
                    onClicked: Detection.input = Detection.Raw
                }
                PanelButton {
                    label: qsTr("Processed")
                    active: Detection.input === Detection.Processed
                    onClicked: Detection.input = Detection.Processed
                }
                Item { width: Theme.spacing; height: 1 } // gap between the two pairs
                PanelButton {
                    label: qsTr("Live")
                    active: !view.showDetectedFrame
                    onClicked: view.showDetectedFrame = false
                }
                PanelButton {
                    label: qsTr("Detected")
                    active: view.showDetectedFrame
                    onClicked: view.showDetectedFrame = true
                }
            }
        }

        Label {
            anchors.centerIn: parent
            width: parent.width * Theme.messageWidthFraction
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            visible: Detection.state === Detection.Loading || Detection.state === Detection.Failed
            text: Detection.state === Detection.Loading ? qsTr("Loading model…")
                                                        : qsTr("Detection unavailable: %1").arg(Detection.error)
            color: Detection.state === Detection.Failed ? Theme.error : Theme.text
            font.pixelSize: Theme.messageSize
            padding: Theme.spacing
            background: Rectangle {
                color: Theme.background
                opacity: Theme.captionOpacity
                radius: Theme.radius
            }
        }
    }

    DetectionPane {
        id: detectionPane

        x: 0
        y: 0
        z: 3
        width: view.width
        height: view.height
        opacity: 0 // shown only in Detection mode
    }

    StreamPane {
        id: rawPane

        x: 0
        y: 0
        z: 0
        width: view.halfWidth
        height: view.height
        stream: FrameView.Raw
        caption: qsTr("Raw")
    }

    StreamPane {
        id: processedPane

        x: view.halfWidth + Theme.paneGap
        y: 0
        z: 1 // drawn over raw in overlay mode
        width: view.halfWidth
        height: view.height
        stream: FrameView.Processed
        caption: qsTr("Processed")
    }

    states: [ // the base state "" is side by side
        State {
            name: "detection"
            when: view.mode === CompareView.ViewMode.Detection

            PropertyChanges {
                target: rawPane
                width: view.width
                opacity: 0
            }
            PropertyChanges {
                target: processedPane
                x: 0
                width: view.width
                opacity: 0
            }
            PropertyChanges {
                target: detectionPane
                opacity: 1
            }
        },
        State {
            name: "overlay"
            when: view.mode === CompareView.ViewMode.Overlay

            PropertyChanges {
                target: rawPane
                width: view.width
                captioned: false
            }
            PropertyChanges {
                target: processedPane
                x: 0
                width: view.width
                opacity: view.overlayOpacity
                captioned: false
            }
        },
        State {
            name: "pictureInPicture"
            when: view.mode === CompareView.ViewMode.PictureInPicture

            PropertyChanges {
                target: processedPane
                x: 0
                width: view.width
            }
            PropertyChanges {
                target: rawPane
                z: 2 // the inset sits on top
                // Scaled down by pipScale and kept pipInset away from the
                // bottom and right edges.
                width: view.width * Theme.pipScale
                height: view.height * Theme.pipScale
                x: view.width - rawPane.width - Theme.pipInset
                y: view.height - rawPane.height - Theme.pipInset
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            properties: "x,y,width,height,opacity"
            duration: Theme.transitionMs
            easing.type: Easing.InOutQuad
        }
    }
}
