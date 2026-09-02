import QtQuick
import QtQuick.Controls.Basic
import PiVision
import PiVision.Display

// The raw and processed images of the pipeline, compared three ways:
//   SideBySide        raw on the left, processed on the right
//   Overlay           processed drawn over raw, faded by overlayOpacity
//   PictureInPicture  processed fills the view, raw inset in the bottom-right corner
// Switching modes animates the panes from one layout to the other.
//
// Overlay lines up because the processed image is a downscale of the raw one
// with the same aspect ratio, and both are fitted to the same pane.
Item {
    id: view

    enum ViewMode {
        SideBySide,
        Overlay,
        PictureInPicture
    }

    property int mode: CompareView.ViewMode.SideBySide
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
