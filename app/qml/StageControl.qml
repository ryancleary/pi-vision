import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import PiVision.Display

// One processing stage: an on/off switch plus a slider per parameter.
// Main.qml creates one of these at runtime for each entry in stages.json.
ColumnLayout {
    id: control

    required property string stageId
    required property string label
    required property bool active
    // [ { name, label, value, min, max, step } ]
    required property var parameters

    Layout.fillWidth: true
    spacing: 2

    Switch {
        id: toggle

        Layout.fillWidth: true
        text: control.label
        checked: control.active
        onToggled: ProcessingControl.setEnabled(control.stageId, checked)
    }

    Repeater {
        model: control.parameters

        delegate: ColumnLayout {
            id: parameter

            required property var modelData

            Layout.fillWidth: true
            Layout.leftMargin: 12
            spacing: 0
            enabled: toggle.checked

            Label {
                // Whole steps show as integers, fractional ones with one decimal.
                text: "%1: %2".arg(parameter.modelData.label)
                              .arg(parameter.modelData.step < 1 ? slider.value.toFixed(1)
                                                                : Math.round(slider.value))
                opacity: parameter.enabled ? 1.0 : 0.5
                font.pixelSize: 12
            }

            Slider {
                id: slider

                Layout.fillWidth: true
                from: parameter.modelData.min
                to: parameter.modelData.max
                stepSize: parameter.modelData.step
                snapMode: Slider.SnapAlways
                value: parameter.modelData.value
                onMoved: ProcessingControl.setParameter(control.stageId, parameter.modelData.name, value)
            }
        }
    }
}
