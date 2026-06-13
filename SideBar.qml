import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhScene 1.0

Rectangle {
    id: sidebar
    color: "#151515"
    border.color: "#2A2A2A"

    required property SceneManager sceneModel

    signal newModelRequest()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Text {
            text: qsTr("Scenes")
            color: "white"
            font.bold: true
        }

        ListView {
            id: sceneListView
            model: sceneModel
            clip: true
            spacing: 8
            Layout.fillWidth: true
            Layout.fillHeight: true
            delegate: Rectangle {
                width: parent.width
                height: 56
                radius: 6
                color: index === sceneModel.activeIndex ? "#2D2D2D" : "#171717"
                border.color: index === sceneModel.activeIndex ? "#86C0FF" : "#333"
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    onClicked: sceneModel.activeIndex = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    Text {
                        text: name
                        color: "white"
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                    }

                    Button {
                        text: qsTr("Remove")
                        onClicked: sceneModel.removeScene(index)
                        background: Rectangle { color: "#3A3A3A"; radius: 6 }
                    }
                }
            }
        }

        Button {
            text: qsTr("+")
            onClicked: sidebar.newModelRequest()
            background: Rectangle { color: "#3A3A3A"; radius: 6 }
        }

        GroupBox {
            title: qsTr("Active Scene")
            Layout.fillWidth: true
            enabled: sceneModel.activeScene !== null

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Visible") }
                    Switch {
                        checked: sceneModel.activeScene ? sceneModel.activeScene.visible : false
                        onCheckedChanged: if (sceneModel.activeScene) sceneModel.activeScene.visible = checked
                    }
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Offset X") }
                    SpinBox {
                        from: -1000
                        to: 1000
                        stepSize: 10
                        value: sceneModel.activeScene ? sceneModel.activeScene.sceneOffset.x : 0
                        enabled: sceneModel.activeScene !== null
                        onValueChanged: {
                            if (!sceneModel.activeScene) return
                            var o = sceneModel.activeScene.sceneOffset
                            sceneModel.activeScene.sceneOffset = Qt.vector3d(value, o.y, o.z)
                        }
                    }
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Offset Y") }
                    SpinBox {
                        from: -1000
                        to: 1000
                        stepSize: 10
                        value: sceneModel.activeScene ? sceneModel.activeScene.sceneOffset.y : 0
                        enabled: sceneModel.activeScene !== null
                        onValueChanged: {
                            if (!sceneModel.activeScene) return
                            var o = sceneModel.activeScene.sceneOffset
                            sceneModel.activeScene.sceneOffset = Qt.vector3d(o.x, value, o.z)
                        }
                    }
                }

                Button {
                    text: qsTr("Reset Offset")
                    enabled: sceneModel.activeScene !== null
                    onClicked: if (sceneModel.activeScene) sceneModel.activeScene.sceneOffset = Qt.vector3d(0, 0, 0)
                    background: Rectangle { color: "#3A3A3A"; radius: 6 }
                }
            }
        }
    }
}
