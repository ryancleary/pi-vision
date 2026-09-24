pragma Singleton
import QtQuick

// Colors, sizes and timings shared by every view, so a change is made once.
QtObject {
    // Colors
    readonly property color background: "#1b1f24" // same as the boot splash
    readonly property color panel: "#262b31"
    readonly property color control: "#343a42"
    readonly property color stroke: "#4a525c"
    readonly property color text: "#e6e6e6"
    readonly property color muted: "#9aa4ad"
    readonly property color accent: "#3cc8ff"
    readonly property color error: "#ff8a80"

    // Text
    readonly property int fontSize: 12
    readonly property int headingSize: 13
    readonly property int messageSize: 18
    readonly property real messageWidthFraction: 0.8 // long messages wrap at 80% of the view

    // Layout
    readonly property int margin: 12
    readonly property int spacing: 8
    readonly property int sidePanelWidth: 280
    readonly property int tabHeight: 36
    readonly property int tabIndicatorHeight: 2
    readonly property int metricSpacing: 4
    readonly property int sourceBoxWidth: 340
    readonly property int radius: 5
    readonly property int borderWidth: 1

    // Buttons
    readonly property int buttonWidth: 96
    readonly property int buttonHeight: 28
    readonly property int buttonGap: 6

    // Compare view
    readonly property int paneGap: 12
    readonly property real pipScale: 0.28 // inset size as a fraction of the view
    readonly property int pipInset: 12    // inset distance from the view's edges
    readonly property int sliderWidth: 140
    readonly property int captionPadding: 4
    readonly property real captionOpacity: 0.7
    readonly property int transitionMs: 225

    // Detection
    readonly property color detectionBox: "#ffd23c"   // stands out on most scenes
    readonly property color detectionLabelText: "#1b1f24"
    readonly property color modelPadding: "#5a6470"   // what the model sees as padding
    readonly property int detectionBoxWidth: 2

    // Messages
    readonly property int toastMs: 3000
}
