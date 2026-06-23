import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BvhViewer
import BvhScene

Rectangle {
    id: sidebar
    color: palette.window
    border.color: palette.mid

    required property SceneManager sceneManager

    signal newModelRequest()

    readonly property int sidebarTabScenes: 0
    readonly property int sidebarTabSettings: 1

    property int sidebarTab: sidebarTabScenes

    function selectSceneAt(index, tab) {
        if (index < 0 || index >= sceneManager.count()) {
            return
        }

        sidebarTab = sidebarTabScenes

        if (sceneManager.activeIndex !== index) {
            sceneManager.activeIndex = index
        }
        activeScenePanel.currentTab = tab
    }

    function deselectScene() {
        sceneManager.activeIndex = -1
    }

    function toggleSceneAt(index, tab) {
        if (index < 0 || index >= sceneManager.count()) {
            return
        }

        if (sceneManager.activeIndex === index) {
            deselectScene()
        } else {
            selectSceneAt(index, tab)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        TabBar {
            id: sidebarTabBar
            Layout.fillWidth: true
            currentIndex: sidebar.sidebarTab
            onCurrentIndexChanged: {
                if (currentIndex !== sidebar.sidebarTab) {
                    sidebar.sidebarTab = currentIndex
                }
            }

            TabButton { text: qsTr("Scenes") }
            TabButton { text: qsTr("Settings") }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: sidebar.sidebarTab

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        id: sceneListView
                        anchors.fill: parent
                        model: sceneManager
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

                            TapHandler {
                                onTapped: sidebar.deselectScene()
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
                            color: index === sidebar.sceneManager.activeIndex ? palette.highlight : palette.base
                            border.color: index === sidebar.sceneManager.activeIndex ? palette.highlight : palette.mid
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
                                        color: index === sidebar.sceneManager.activeIndex ? palette.highlightedText : palette.text
                                        elide: Text.ElideRight
                                        verticalAlignment: Text.AlignVCenter
                                    }

                                    TapHandler {
                                        onTapped: sidebar.toggleSceneAt(sceneRow.index, 0)
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
                    sceneManager: sidebar.sceneManager
                }
            }

            ScrollView {
                id: settingsScroll
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                SceneSettingsPanel {
                    width: settingsScroll.availableWidth
                    viewportSettings: sidebar.sceneManager.viewportSettings
                    cameraSettings: sidebar.sceneManager.cameraSettings
                }
            }
        }
    }
}
