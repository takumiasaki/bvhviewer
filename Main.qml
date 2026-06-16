import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhViewer
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
        Menu {
            title: qsTr("&View")
            Action {
                text: qsTr("Show &Floor Shadows")
                checkable: true
                checked: sceneManager.floorShadowsEnabled
                onTriggered: sceneManager.floorShadowsEnabled = checked
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

            sceneManager: sceneManager

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

            TimelineBar {
                sceneManager: sceneManager
            }
        }
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
                  : (sceneManager.sceneCount > 0
                     ? (sceneManager.activeScene
                        ? qsTr("Loaded %1").arg(sceneManager.activeScene.source.toString())
                        : qsTr("%1 scene(s) loaded").arg(sceneManager.sceneCount))
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
