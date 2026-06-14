import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhScene 1.0

Rectangle {
    id: sidebar
    color: palette.window
    border.color: palette.mid

    required property SceneManager sceneModel

    signal newModelRequest()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Label {
            text: qsTr("Scenes")
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
                height: 40
                radius: 6
                color: index === sceneModel.activeIndex ? palette.highlight : palette.base
                border.color: index === sceneModel.activeIndex ? palette.highlight : palette.mid
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    onClicked: sceneModel.activeIndex = index
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 8

                    CheckBox {
                        checked: skeleton.visible
                        onToggled: skeleton.visible = checked
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Show in 3D view")
                    }

                    Label {
                        text: name
                        color: index === sceneModel.activeIndex ? palette.highlightedText : palette.text
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Button {
            text: qsTr("+")
            onClicked: sidebar.newModelRequest()
        }

        Rectangle {
            id: activeScenePanel
            visible: sceneModel.activeIndex >= 0
            Layout.fillWidth: true
            radius: 6
            color: palette.base
            border.color: palette.mid
            border.width: 1
            implicitHeight: activeSceneLayout.implicitHeight + 16

            ColumnLayout {
                id: activeSceneLayout
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Label {
                        text: qsTr("Active Scene")
                        font.bold: true
                        color: palette.text
                    }

                    Item { Layout.fillWidth: true }

                    ToolButton {
                        text: "\u00d7"
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Deselect scene")
                        onClicked: sceneModel.activeIndex = -1
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
                        onValueChanged: {
                            if (!sceneModel.activeScene) return
                            var o = sceneModel.activeScene.sceneOffset
                            sceneModel.activeScene.sceneOffset = Qt.vector3d(o.x, value, o.z)
                        }
                    }
                }

                Button {
                    text: qsTr("Reset Offset")
                    onClicked: if (sceneModel.activeScene) sceneModel.activeScene.sceneOffset = Qt.vector3d(0, 0, 0)
                }

                Button {
                    text: qsTr("Remove")
                    onClicked: sceneModel.removeScene(sceneModel.activeIndex)
                }
            }
        }
    }
}
