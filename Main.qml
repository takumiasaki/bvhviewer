import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhScene

ApplicationWindow {
    id: window
    visible: true
    width: 960
    height: 640
    title: qsTr("BVH Viewer")
    color: palette.window

    SceneManager {
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

        SideBar {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: 240

            sceneModel: sceneManager

            onNewModelRequest: fileDialog.open()
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                id: canvasArea
                Layout.fillWidth: true
                Layout.fillHeight: true

                Bvh3DView {
                    anchors.fill: parent
                    sceneManager: sceneManager
                }
            }

            Rectangle {
                id: animationController
                Layout.fillWidth: true
                height: 56
                color: palette.base
                border.color: palette.mid
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
                        onClicked: sceneManager.toggle()
                    }

                    Button {
                        id: resetButton
                        text: qsTr("Reset")
                        enabled: sceneManager.activeScene && sceneManager.activeScene.valid && !sceneManager.playing
                        visible: sceneManager.activeScene && sceneManager.activeScene.valid
                        onClicked: {
                            sceneManager.animationTime = 0
                            sceneManager.currentFrame = 0
                        }
                    }

                    Slider {
                        id: frameSlider
                        Layout.preferredWidth: parent.width * 0.6
                        from: 0
                        to: sceneManager.frameCount > 0 ? Math.max(0, sceneManager.frameCount - 1) : 0
                        stepSize: 1
                        value: sceneManager.currentFrame >= 0 ? sceneManager.currentFrame : 0
                        visible: sceneManager.activeScene && sceneManager.frameCount > 0
                        onMoved: sceneManager.currentFrame = Math.round(value)
                    }

                    Label {
                        visible: sceneManager.activeScene && sceneManager.frameCount > 0
                        text: sceneManager.activeScene
                              ? qsTr("Frame %1 / %2").arg(sceneManager.currentFrame).arg(Math.max(0, sceneManager.frameCount - 1))
                              : ""
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
        to: 3600
        duration: 3600 * 1000
        loops: Animation.Infinite
        running: sceneManager.playing
    }

    footer: Rectangle {
        id: statusBar
        Layout.fillWidth: true
        height: 30
        color: palette.window
        border.color: palette.mid
        border.width: 1

        Label {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 14
            text: sceneManager.lastError !== ""
                  ? sceneManager.lastError
                  : (sceneManager.activeScene
                     ? qsTr("Loaded %1").arg(sceneManager.activeScene.source.toString())
                     : qsTr("No BVH loaded. Use File > Open or specify a .bvh file on the command line."))
            color: sceneManager.lastError !== "" ? SemanticColors.error : palette.windowText
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
