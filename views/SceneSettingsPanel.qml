import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhViewer
import BvhScene

ColumnLayout {
    id: root

    required property ViewportSettings viewportSettings

    spacing: 8

    function openGroundColorDialog() {
        colorDialog.selectedColor = viewportSettings.groundColor
        colorDialog.open()
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
}
