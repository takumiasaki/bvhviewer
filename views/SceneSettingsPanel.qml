import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhViewer
import BvhScene

ColumnLayout {
    id: root

    required property ViewportSettings viewportSettings
    required property CameraSettings cameraSettings

    readonly property bool groundSizeEnabled: viewportSettings.groundShape !== ViewportSettings.None

    property bool suppressSizeWrite: false

    spacing: 8

    function openGroundColorDialog() {
        colorDialog.selectedColor = viewportSettings.groundColor
        colorDialog.open()
    }

    function syncSizeSpinBoxes() {
        suppressSizeWrite = true
        groundSizeXSpinBox.value = viewportSettings.groundSizeX
        groundSizeYSpinBox.value = viewportSettings.groundSizeY
        groundSizeZSpinBox.value = viewportSettings.groundSizeZ
        suppressSizeWrite = false
    }

    ColorDialog {
        id: colorDialog
        title: qsTr("Ground color")
        onAccepted: {
            viewportSettings.groundColor = selectedColor
        }
    }

    GroupBox {
        title: qsTr("Ground")
        Layout.fillWidth: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: qsTr("Color")
                    Layout.alignment: Qt.AlignVCenter
                }

                ColorSwatch {
                    color: root.viewportSettings.groundColor
                    Layout.alignment: Qt.AlignVCenter

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.openGroundColorDialog()
                    }
                }

                Item { Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Label {
                    text: qsTr("Shape")
                    Layout.alignment: Qt.AlignVCenter
                }

                ComboBox {
                    id: groundShapeCombo
                    Layout.fillWidth: true
                    model: [
                        qsTr("None"),
                        qsTr("Rectangle"),
                        qsTr("Ellipse")
                    ]
                    currentIndex: root.viewportSettings.groundShape
                    onActivated: root.viewportSettings.groundShape = currentIndex
                }
            }

            RowLayout {
                spacing: 8
                enabled: root.groundSizeEnabled

                Label { text: qsTr("Size X") }
                SpinBox {
                    id: groundSizeXSpinBox
                    from: 1
                    to: 10000
                    stepSize: 10
                    value: root.viewportSettings.groundSizeX
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Width (rectangle) or horizontal diameter (ellipse)")
                    onValueChanged: {
                        if (root.suppressSizeWrite) {
                            return
                        }
                        root.viewportSettings.groundSizeX = value
                    }
                }
            }

            RowLayout {
                spacing: 8
                enabled: root.groundSizeEnabled

                Label { text: qsTr("Size Y") }
                SpinBox {
                    id: groundSizeYSpinBox
                    from: 1
                    to: 10000
                    stepSize: 1
                    value: root.viewportSettings.groundSizeY
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Thickness extending downward from Y = 0")
                    onValueChanged: {
                        if (root.suppressSizeWrite) {
                            return
                        }
                        root.viewportSettings.groundSizeY = value
                    }
                }
            }

            RowLayout {
                spacing: 8
                enabled: root.groundSizeEnabled

                Label { text: qsTr("Size Z") }
                SpinBox {
                    id: groundSizeZSpinBox
                    from: 1
                    to: 10000
                    stepSize: 10
                    value: root.viewportSettings.groundSizeZ
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Depth (rectangle) or depth diameter (ellipse)")
                    onValueChanged: {
                        if (root.suppressSizeWrite) {
                            return
                        }
                        root.viewportSettings.groundSizeZ = value
                    }
                }
            }

            Button {
                text: qsTr("Reset to default")
                Layout.fillWidth: true
                onClicked: root.viewportSettings.resetGroundToDefault()
            }
        }
    }

    CheckBox {
        text: qsTr("Floor shadows")
        checked: root.viewportSettings.floorShadowsEnabled
        onToggled: root.viewportSettings.floorShadowsEnabled = checked
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Cast shadows from skeletons onto the ground")
    }

    GroupBox {
        title: qsTr("Camera")
        Layout.fillWidth: true

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                RadioButton {
                    id: manualModeRadio
                    text: qsTr("Manual")
                    checked: root.cameraSettings.mode === CameraSettings.Manual
                    onToggled: {
                        if (checked) {
                            root.cameraSettings.mode = CameraSettings.Manual
                        }
                    }
                }

                RadioButton {
                    id: autoModeRadio
                    text: qsTr("Auto")
                    checked: root.cameraSettings.mode === CameraSettings.Auto
                    onToggled: {
                        if (checked) {
                            root.cameraSettings.mode = CameraSettings.Auto
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Auto: the camera follows the target only while playback is running.")
                color: palette.placeholderText
                font.pixelSize: Math.max(11, font.pixelSize - 1)
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: qsTr("Frame now")
                    Layout.fillWidth: true
                    onClicked: root.cameraSettings.requestFrameNow(false)
                    ToolTip.visible: hovered
                    ToolTip.text: qsTr("Frame the current target in view (F)")
                }

                Button {
                    text: qsTr("Reset view")
                    Layout.fillWidth: true
                    onClicked: root.cameraSettings.requestResetView()
                }
            }
        }
    }

    Connections {
        target: root.cameraSettings
        function onModeChanged() {
            manualModeRadio.checked = root.cameraSettings.mode === CameraSettings.Manual
            autoModeRadio.checked = root.cameraSettings.mode === CameraSettings.Auto
        }
    }

    Connections {
        target: root.viewportSettings
        function onGroundShapeChanged() {
            groundShapeCombo.currentIndex = root.viewportSettings.groundShape
        }
        function onGroundSizeXChanged() {
            root.syncSizeSpinBoxes()
        }
        function onGroundSizeYChanged() {
            root.syncSizeSpinBoxes()
        }
        function onGroundSizeZChanged() {
            root.syncSizeSpinBoxes()
        }
    }
}
