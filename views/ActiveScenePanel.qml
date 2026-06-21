import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhViewer
import BvhScene

Rectangle {
    id: root

    required property SceneManager sceneManager

    readonly property int tabOffset: 0
    readonly property int tabAppearance: 1
    readonly property int tabRemove: 2

    property int currentTab: tabOffset
    property int colorPickTarget: 0

    readonly property bool panelVisible: sceneManager.activeIndex >= 0

    visible: panelVisible
    Layout.fillWidth: true
    Layout.preferredHeight: panelVisible ? implicitHeight : 0
    radius: 6
    color: palette.base
    border.color: palette.mid
    border.width: 1
    implicitHeight: panelLayout.implicitHeight + 16

    onCurrentTabChanged: {
        if (tabBar.currentIndex !== currentTab) {
            tabBar.currentIndex = currentTab
        }
    }

    function openJointColorDialog() {
        if (!sceneManager.activeScene) {
            return
        }
        colorPickTarget = 0
        colorDialog.title = qsTr("Joint color")
        colorDialog.selectedColor = sceneManager.activeScene.jointColor
        colorDialog.open()
    }

    function openBoneColorDialog() {
        if (!sceneManager.activeScene) {
            return
        }
        colorPickTarget = 1
        colorDialog.title = qsTr("Bone color")
        colorDialog.selectedColor = sceneManager.activeScene.customBoneColor
        colorDialog.open()
    }

    ColorDialog {
        id: colorDialog
        onAccepted: {
            if (!sceneManager.activeScene) {
                return
            }
            if (colorPickTarget === 0) {
                sceneManager.activeScene.jointColor = selectedColor
            } else {
                sceneManager.activeScene.boneColorMode = appearanceTab.boneModeCustom
                sceneManager.activeScene.customBoneColor = selectedColor
            }
        }
    }

    MessageDialog {
        id: removeConfirmDialog
        title: qsTr("Remove scene")
        text: sceneManager.activeScene
              ? qsTr("Remove \"%1\" from the scene?").arg(sceneManager.activeScene.displayName)
              : ""
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            sceneManager.removeScene(sceneManager.activeIndex)
        }
    }

    ColumnLayout {
        id: panelLayout
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        RowLayout {
            Layout.fillWidth: true
            spacing: 4

            Label {
                text: sceneManager.activeScene
                      ? qsTr("Active Scene · %1").arg(sceneManager.activeScene.displayName)
                      : qsTr("Active Scene")
                font.bold: true
                color: palette.text
                elide: Text.ElideRight
                Layout.fillWidth: true
            }

            ToolButton {
                text: "\u00d7"
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Deselect scene")
                onClicked: sceneManager.activeIndex = -1
            }
        }

        TabBar {
            id: tabBar
            Layout.fillWidth: true
            currentIndex: root.currentTab
            onCurrentIndexChanged: {
                if (currentIndex !== root.currentTab) {
                    root.currentTab = currentIndex
                }
            }

            TabButton { text: qsTr("Offset") }
            TabButton { text: qsTr("Appearance") }
            TabButton { text: qsTr("Remove") }
        }

        StackLayout {
            Layout.fillWidth: true
            currentIndex: root.currentTab

            OffsetTab {
                sceneManager: root.sceneManager
            }

            AppearanceTab {
                id: appearanceTab
                sceneManager: root.sceneManager
                onChooseJointColor: root.openJointColorDialog()
                onChooseBoneColor: root.openBoneColorDialog()
            }

            RemoveTab {
                sceneManager: root.sceneManager
                onRemoveRequested: removeConfirmDialog.open()
            }
        }
    }
}
