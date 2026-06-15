import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BvhScene 1.0

Rectangle {
    id: sidebar
    color: palette.window
    border.color: palette.mid

    required property SceneManager sceneModel

    signal newModelRequest()

    function selectSceneAt(index, tab) {
        activeScenePanel.selectScene(index, tab)
    }

    function deselectScene() {
        activeScenePanel.deselectScene()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Label {
            text: qsTr("Scenes")
            font.bold: true
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: sceneListView
                anchors.fill: parent
                model: sceneModel
                clip: true
                spacing: 8

                readonly property int delegateHeight: 40

                TapHandler {
                    onTapped: (eventPoint) => {
                        const pos = eventPoint.position
                        if (sceneListView.indexAt(pos.x, pos.y + sceneListView.contentY) < 0) {
                            sidebar.deselectScene()
                        }
                    }
                }

                footer: Item {
                    width: sceneListView.width
                    height: Math.max(0, sceneListView.height - (sceneListView.count > 0
                        ? sceneListView.count * sceneListView.delegateHeight
                          + (sceneListView.count - 1) * sceneListView.spacing
                        : 0))

                    MouseArea {
                        anchors.fill: parent
                        onClicked: sidebar.deselectScene()
                    }
                }

                delegate: Rectangle {
                    id: sceneRow
                    required property int index
                    required property string name
                    required property BvhSkeletonItem skeleton

                    width: sceneListView.width
                    height: sceneListView.delegateHeight
                    radius: 6
                    color: index === sidebar.sceneModel.activeIndex ? palette.highlight : palette.base
                    border.color: index === sidebar.sceneModel.activeIndex ? palette.highlight : palette.mid
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 8

                        CheckBox {
                            checked: sceneRow.skeleton.visible
                            onToggled: sceneRow.skeleton.visible = checked
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Show in 3D view")
                        }

                        DualColorSwatch {
                            jointColor: sceneRow.skeleton.jointColor
                            boneColor: sceneRow.skeleton.boneColor
                            onClicked: sidebar.selectSceneAt(sceneRow.index, 1)
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Label {
                                anchors.fill: parent
                                text: sceneRow.name
                                color: index === sidebar.sceneModel.activeIndex ? palette.highlightedText : palette.text
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: sidebar.selectSceneAt(sceneRow.index, 0)
                            }
                        }
                    }
                }
            }
        }

        Button {
            text: qsTr("+")
            onClicked: sidebar.newModelRequest()
        }

        ActiveScenePanel {
            id: activeScenePanel
            sceneModel: sidebar.sceneModel
        }
    }
}
