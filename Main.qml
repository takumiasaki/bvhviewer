import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhViewer 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 960
    height: 640
    title: qsTr("BVH Viewer")
    color: "#121212"

    // playback controlled by sceneManager.playing

    BvhSceneListModel {
        id: sceneManager
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Open BVH File")
        nameFilters: [qsTr("BVH Files (*.bvh)")]
        onAccepted: {
            sceneManager.loadScene(fileDialog.selectedFile)
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")
            Action {
                id: openAction
                text: qsTr("&Open...")
                shortcut: StandardKey.Open
                onTriggered: fileDialog.open()
            }
            Action {
                text: qsTr("E&xit")
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        /* Left: sidebar with scene list */
        Rectangle {
            id: sidebar
            Layout.preferredWidth: 240
            Layout.fillHeight: true
            color: "#151515"
            border.color: "#2A2A2A"

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
                    model: sceneManager
                    clip: true
                    spacing: 8
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    delegate: Rectangle {
                        width: parent.width
                        height: 56
                        radius: 6
                        color: index === sceneManager.activeIndex ? "#2D2D2D" : "#171717"
                        border.color: index === sceneManager.activeIndex ? "#86C0FF" : "#333"
                        border.width: 1

                        MouseArea {
                            anchors.fill: parent
                            onClicked: sceneManager.activeIndex = index
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
                                onClicked: sceneManager.removeScene(index)
                                background: Rectangle { color: "#3A3A3A"; radius: 6 }
                            }
                        }
                    }
                }
            }
        }

        /* Right: main area with canvas and bottom controls */
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                id: canvasArea
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#101010"

                BvhViewItem {
                    id: bvhView
                    anchors.fill: parent
                    sceneManager: sceneManager
                    boneColor: "white"
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 56
                color: "#1F1F1F"
                border.color: "#2A2A2A"
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    Button {
                        id: playButton
                        text: sceneManager.playing ? qsTr("Pause") : qsTr("Play")
                        enabled: sceneManager.activeScene && sceneManager.activeScene.frameCount > 1
                        visible: sceneManager.activeScene && sceneManager.activeScene.valid
                        background: Rectangle { color: "#2D2D2D"; radius: 8 }
                        font.bold: true
                        onClicked: sceneManager.toggle()
                    }

                    Button {
                        id: resetButton
                        text: qsTr("Reset")
                        enabled: sceneManager.activeScene && sceneManager.activeScene.valid && !sceneManager.playing
                        visible: sceneManager.activeScene && sceneManager.activeScene.valid
                        background: Rectangle { color: "#2D2D2D"; radius: 8 }
                        font.bold: true
                        onClicked: {
                            sceneManager.animationTime = 0
                            if (sceneManager.activeScene) sceneManager.activeScene.currentFrame = 0
                        }
                    }

                    Slider {
                        id: frameSlider
                        Layout.preferredWidth: parent.width * 0.6
                        from: 0
                        to: sceneManager.activeScene ? Math.max(0, sceneManager.activeScene.frameCount - 1) : 0
                        stepSize: 1
                        value: sceneManager.activeScene ? sceneManager.activeScene.currentFrame : 0
                        visible: sceneManager.activeScene && sceneManager.activeScene.frameCount > 0
                        onMoved: if (sceneManager.activeScene) sceneManager.activeScene.currentFrame = Math.round(value)
                    }

                    Text {
                        visible: sceneManager.activeScene && sceneManager.activeScene.frameCount > 0
                        text: sceneManager.activeScene ? qsTr("Frame %1 / %2").arg(sceneManager.activeScene.currentFrame).arg(Math.max(0, sceneManager.activeScene.frameCount - 1)) : ""
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    NumberAnimation {
        id: playAnimation
        target: sceneManager
        property: "animationTime"
        from: 0
        to: 3600    // 1 hour cycle in seconds
        duration: 3600 * 1000
        loops: Animation.Infinite
        running: sceneManager.playing
    }

    footer: Rectangle {
        id: statusBar
        Layout.fillWidth: true
        height: 30
        color: "#121212"
        border.color: "#2A2A2A"
        border.width: 1

        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 14
            text: sceneManager.activeScene ? (sceneManager.activeScene.errorString !== "" ? sceneManager.activeScene.errorString : qsTr("Loaded %1").arg(sceneManager.activeScene.source.toString())) : qsTr("No BVH loaded. Use File > Open or specify a .bvh file on the command line.")
            color: "white"
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            width: parent.width - 28
        }
    }

    Component.onCompleted: {
        if (Qt.application.arguments.length > 1) {
            var arg = Qt.application.arguments[1];
            if (arg.toLowerCase().endsWith(".bvh")) {
                sceneManager.loadScene(QUrl.fromLocalFile(arg));
            }
        }
    }
}
