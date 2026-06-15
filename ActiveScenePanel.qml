import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhScene 1.0
import "SceneColorUtils.js" as ColorUtils

Rectangle {
    id: root

    required property SceneManager sceneModel

    readonly property int tabOffset: 0
    readonly property int tabAppearance: 1
    readonly property int tabRemove: 2

    property int currentTab: tabOffset
    property bool suppressOffsetWrite: false
    property real boneTone: ColorUtils.defaultBoneTone
    property int colorPickTarget: 0

    readonly property bool panelVisible: sceneModel.activeIndex >= 0

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

    function selectScene(index, tab) {
        if (index < 0 || index >= sceneModel.count()) {
            return
        }

        if (sceneModel.activeIndex !== index) {
            sceneModel.activeIndex = index
        }
        currentTab = tab
    }

    function deselectScene() {
        sceneModel.activeIndex = -1
    }

    function syncAppearanceUiFromSkeleton() {
        if (!sceneModel.activeScene) {
            return
        }
        boneTone = sceneModel.activeScene.colorsLinked
                     ? ColorUtils.defaultBoneTone
                     : ColorUtils.estimateBoneTone(sceneModel.activeScene.jointColor,
                                                   sceneModel.activeScene.boneColor)
    }

    function resetOffsetToDefault() {
        if (!sceneModel.activeScene) {
            return
        }
        suppressOffsetWrite = true
        sceneModel.activeScene.userSceneOffset = Qt.vector3d(0, 0, 0)
        suppressOffsetWrite = false
    }

    function resetAppearanceToDefault() {
        if (sceneModel.activeIndex < 0) {
            return
        }
        sceneModel.resetSkeletonColors(sceneModel.activeIndex)
        syncAppearanceUiFromSkeleton()
    }

    function applyBoneTone(tone) {
        if (!sceneModel.activeScene || sceneModel.activeScene.colorsLinked) {
            return
        }
        sceneModel.activeScene.boneColor = ColorUtils.boneColorFromTone(
                    sceneModel.activeScene.jointColor, tone)
    }

    function openJointColorDialog() {
        if (!sceneModel.activeScene) {
            return
        }
        colorPickTarget = 0
        colorDialog.title = qsTr("Joint color")
        colorDialog.selectedColor = sceneModel.activeScene.jointColor
        colorDialog.open()
    }

    function openBoneColorDialog() {
        if (!sceneModel.activeScene) {
            return
        }
        colorPickTarget = 1
        colorDialog.title = qsTr("Bone color")
        colorDialog.selectedColor = sceneModel.activeScene.boneColor
        colorDialog.open()
    }

    ColorDialog {
        id: colorDialog
        onAccepted: {
            if (!sceneModel.activeScene) {
                return
            }
            if (colorPickTarget === 0) {
                sceneModel.activeScene.jointColor = selectedColor
                if (!sceneModel.activeScene.colorsLinked) {
                    applyBoneTone(boneTone)
                }
            } else {
                sceneModel.activeScene.colorsLinked = false
                sceneModel.activeScene.boneColor = selectedColor
                boneTone = ColorUtils.estimateBoneTone(sceneModel.activeScene.jointColor,
                                                       selectedColor)
            }
        }
    }

    MessageDialog {
        id: removeConfirmDialog
        title: qsTr("Remove scene")
        text: sceneModel.activeScene
              ? qsTr("Remove \"%1\" from the scene?").arg(sceneModel.activeScene.displayName)
              : ""
        buttons: MessageDialog.Yes | MessageDialog.No
        onAccepted: {
            sceneModel.removeScene(sceneModel.activeIndex)
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
                text: sceneModel.activeScene
                      ? qsTr("Active Scene · %1").arg(sceneModel.activeScene.displayName)
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
                onClicked: root.deselectScene()
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

            ColumnLayout {
                spacing: 8

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Offset X") }
                    SpinBox {
                        id: offsetXSpinBox
                        from: -1000
                        to: 1000
                        stepSize: 10
                        value: sceneModel.activeScene ? sceneModel.activeScene.userSceneOffset.x : 0
                        onValueChanged: {
                            if (root.suppressOffsetWrite || !sceneModel.activeScene) {
                                return
                            }
                            const o = sceneModel.activeScene.userSceneOffset
                            sceneModel.activeScene.userSceneOffset = Qt.vector3d(value, o.y, o.z)
                        }
                    }
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Offset Y") }
                    SpinBox {
                        id: offsetYSpinBox
                        from: -1000
                        to: 1000
                        stepSize: 10
                        value: sceneModel.activeScene ? sceneModel.activeScene.userSceneOffset.y : 0
                        onValueChanged: {
                            if (root.suppressOffsetWrite || !sceneModel.activeScene) {
                                return
                            }
                            const o = sceneModel.activeScene.userSceneOffset
                            sceneModel.activeScene.userSceneOffset = Qt.vector3d(o.x, value, o.z)
                        }
                    }
                }

                RowLayout {
                    spacing: 8
                    Label { text: qsTr("Offset Z") }
                    SpinBox {
                        id: offsetZSpinBox
                        from: -1000
                        to: 1000
                        stepSize: 10
                        value: sceneModel.activeScene ? sceneModel.activeScene.userSceneOffset.z : 0
                        onValueChanged: {
                            if (root.suppressOffsetWrite || !sceneModel.activeScene) {
                                return
                            }
                            const o = sceneModel.activeScene.userSceneOffset
                            sceneModel.activeScene.userSceneOffset = Qt.vector3d(o.x, o.y, value)
                        }
                    }
                }

                Button {
                    text: qsTr("Reset to default")
                    Layout.fillWidth: true
                    onClicked: root.resetOffsetToDefault()
                }
            }

            ColumnLayout {
                spacing: 8

                GridLayout {
                    columns: 3
                    columnSpacing: 8
                    rowSpacing: 8
                    Layout.fillWidth: true

                    Label {
                        text: qsTr("Joint")
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Rectangle {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        radius: 2
                        color: sceneModel.activeScene ? sceneModel.activeScene.jointColor : "white"
                        border.color: palette.mid
                        border.width: 1
                    }

                    Button {
                        text: qsTr("Choose color…")
                        enabled: sceneModel.activeScene
                        onClicked: root.openJointColorDialog()
                    }
                }

                CheckBox {
                    text: qsTr("Same color for joints and bones")
                    enabled: sceneModel.activeScene
                    checked: sceneModel.activeScene ? sceneModel.activeScene.colorsLinked : true
                    onToggled: {
                        if (sceneModel.activeScene) {
                            sceneModel.activeScene.colorsLinked = checked
                            if (!checked) {
                                root.applyBoneTone(root.boneTone)
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    enabled: sceneModel.activeScene && !sceneModel.activeScene.colorsLinked
                    opacity: enabled ? 1.0 : 0.45

                    Label {
                        text: qsTr("Bone tone (same hue, adjust lightness)")
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            text: qsTr("Darker")
                            font.pixelSize: 11
                        }

                        Slider {
                            Layout.fillWidth: true
                            from: -100
                            to: 100
                            stepSize: 5
                            value: root.boneTone
                            onMoved: {
                                root.boneTone = value
                                root.applyBoneTone(value)
                            }
                        }

                        Label {
                            text: qsTr("Lighter")
                            font.pixelSize: 11
                        }
                    }

                    GridLayout {
                        columns: 3
                        columnSpacing: 8
                        rowSpacing: 8
                        Layout.fillWidth: true

                        Label {
                            text: qsTr("Bone")
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Rectangle {
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            radius: 2
                            color: sceneModel.activeScene ? sceneModel.activeScene.boneColor : "white"
                            border.color: palette.mid
                            border.width: 1
                        }

                        Button {
                            text: qsTr("Choose color…")
                            onClicked: root.openBoneColorDialog()
                        }
                    }
                }

                Button {
                    text: qsTr("Reset to default")
                    Layout.fillWidth: true
                    enabled: sceneModel.activeIndex >= 0
                    onClicked: root.resetAppearanceToDefault()
                }
            }

            ColumnLayout {
                spacing: 12

                Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: sceneModel.activeScene
                          ? qsTr("Remove \"%1\" from the loaded scenes. This frees memory and cannot be undone.")
                            .arg(sceneModel.activeScene.displayName)
                          : ""
                }

                Button {
                    text: qsTr("Remove scene…")
                    Layout.fillWidth: true
                    enabled: sceneModel.activeIndex >= 0
                    onClicked: removeConfirmDialog.open()
                }
            }
        }
    }

    Connections {
        target: sceneModel
        function onActiveIndexChanged() {
            if (sceneModel.activeIndex < 0) {
                return
            }
            syncOffsetSpinBoxes()
            syncAppearanceUiFromSkeleton()
        }
    }

    Connections {
        target: sceneModel.activeScene
        enabled: sceneModel.activeScene
        function onSceneOffsetChanged() {
            syncOffsetSpinBoxes()
        }
    }

    function syncOffsetSpinBoxes() {
        if (!sceneModel.activeScene) {
            return
        }
        suppressOffsetWrite = true
        offsetXSpinBox.value = sceneModel.activeScene.userSceneOffset.x
        offsetYSpinBox.value = sceneModel.activeScene.userSceneOffset.y
        offsetZSpinBox.value = sceneModel.activeScene.userSceneOffset.z
        suppressOffsetWrite = false
    }
}
