import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import PiVision
import PiVision.Display

// Live pipeline and system numbers, averaged over the last second. Polls only
// while visible, so the hidden tab costs nothing.
ScrollView {
    id: panel

    contentWidth: availableWidth

    Binding {
        target: MetricsMonitor
        property: "running"
        value: panel.visible
    }

    function fps(value) { return qsTr("%1 fps").arg(value.toFixed(1)) }
    function ms(value) { return qsTr("%1 ms").arg(value.toFixed(1)) }
    function perSecond(value) { return qsTr("%1 /s").arg(value.toFixed(1)) }

    component Heading: Label {
        Layout.topMargin: Theme.spacing
        color: Theme.muted
        font.pixelSize: Theme.headingSize
    }

    // A name on the left, its value right-aligned.
    component Metric: RowLayout {
        property alias name: nameLabel.text
        property alias value: valueLabel.text
        property alias valueColor: valueLabel.color

        Layout.fillWidth: true
        spacing: Theme.spacing

        Label {
            id: nameLabel

            Layout.fillWidth: true
            elide: Text.ElideRight
            font.pixelSize: Theme.fontSize
        }

        Label {
            id: valueLabel

            Layout.maximumWidth: panel.availableWidth * 0.6
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSize
        }
    }

    ColumnLayout {
        width: panel.availableWidth
        spacing: Theme.metricSpacing

        Heading { text: qsTr("Frame rate") }
        Metric { name: qsTr("Capture"); value: panel.fps(MetricsMonitor.captureFps) }
        Metric { name: qsTr("Display"); value: panel.fps(MetricsMonitor.displayFps) }

        Heading { text: qsTr("Time per frame") }
        Metric { name: qsTr("Capture to UI"); value: panel.ms(MetricsMonitor.latencyMs) }
        Metric { name: qsTr("All stages"); value: panel.ms(MetricsMonitor.stagesMs) }
        Repeater {
            model: MetricsMonitor.stages

            delegate: Metric {
                required property var modelData

                Layout.leftMargin: Theme.margin
                name: modelData.label
                value: panel.ms(modelData.ms)
            }
        }
        Label {
            Layout.leftMargin: Theme.margin
            visible: MetricsMonitor.stages.length === 0
            text: qsTr("No stages enabled")
            color: Theme.muted
            font.pixelSize: Theme.fontSize
        }

        Heading { text: qsTr("Dropped frames") }
        Metric { name: qsTr("Before processing"); value: panel.perSecond(MetricsMonitor.droppedBeforeProcessing) }
        Metric { name: qsTr("Before display"); value: panel.perSecond(MetricsMonitor.droppedBeforeDisplay) }

        Heading { text: qsTr("System") }
        Metric {
            name: qsTr("CPU (100% = one core)")
            value: qsTr("%1%").arg(Math.round(MetricsMonitor.cpuPercent))
        }
        Metric {
            name: qsTr("Temperature")
            value: MetricsMonitor.hasTemperature ? qsTr("%1 °C").arg(MetricsMonitor.temperature.toFixed(1))
                                                 : qsTr("n/a")
        }
        Metric {
            name: qsTr("Throttling")
            value: !MetricsMonitor.hasThrottle ? qsTr("n/a")
                 : MetricsMonitor.throttleNow.length > 0 ? MetricsMonitor.throttleNow.join(", ")
                 : qsTr("none")
            valueColor: MetricsMonitor.throttleNow.length > 0 ? Theme.error : Theme.text
        }
        Metric {
            visible: MetricsMonitor.hasThrottle
            name: qsTr("Since boot")
            value: MetricsMonitor.throttleSinceBoot.length > 0 ? MetricsMonitor.throttleSinceBoot.join(", ")
                                                              : qsTr("none")
        }
    }
}
