import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import BvhScene 1.0

Rectangle {
    id: root

    required property SceneManager sceneModel

    readonly property int tabOffset: 0
    readonly property int tabAppearance: 1
    readonly property int tabRemove: 2
    readonly property int boneModeSameAsJoint: 0
    readonly property int boneModeToneOffset: 1
    readonly property int boneModeCustom: 2

    property int currentTab: tabOffset
    property bool suppressOffsetWrite: false
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

    function resetOffsetToDefault() {
        if (!sceneModel.activeScene) {
            return
        }
        suppressOffsetWrite = true
        sceneModel.activeScene.sceneOffset = Qt.vector3d(0, 0, 0)
        suppressOffsetWrite = false
    }

    function resetAppearanceToDefault() {
        if (sceneModel.activeIndex < 0) {
            return
        }
        sceneModel.resetSkeletonColors(sceneModel.activeIndex)
    }

    function setBoneColorMode(mode) {
        if (!sceneModel.activeScene) {
            return
        }
        sceneModel.activeScene.boneColorMode = mode
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
        colorDialog.selectedColor = sceneModel.activeScene.customBoneColor
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
            } else {
                sceneModel.activeScene.boneColorMode = root.boneModeCustom
                sceneModel.activeScene.customBoneColor = selectedColor
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
                        value: sceneModel.activeScene ? sceneModel.activeScene.sceneOffset.x : 0
                        onValueChanged: {
                            if (root.suppressOffsetWrite || !sceneModel.activeScene) {
                                return
                            }
                            const o = sceneModel.activeScene.sceneOffset
                            sceneModel.activeScene.sceneOffset = Qt.vector3d(value, o.y, o.z)
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
                        value: sceneModel.activeScene ? sceneModel.activeScene.sceneOffset.y : 0
                        onValueChanged: {
                            if (root.suppressOffsetWrite || !sceneModel.activeScene) {
                                return
                            }
                            const o = sceneModel.activeScene.sceneOffset
                            sceneModel.activeScene.sceneOffset = Qt.vector3d(o.x, value, o.z)
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
                        value: sceneModel.activeScene ? sceneModel.activeScene.sceneOffset.z : 0
                        onValueChanged: {
                            if (root.suppressOffsetWrite || !sceneModel.activeScene) {
                                return
                            }
                            const o = sceneModel.activeScene.sceneOffset
                            sceneModel.activeScene.sceneOffset = Qt.vector3d(o.x, o.y, value)
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
                        Layout.row: 0
                        Layout.column: 0
                        Layout.alignment: Qt.AlignVCenter
                    }

                    Rectangle {
                        Layout.row: 0
                        Layout.column: 1
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        radius: 2
                        color: sceneModel.activeScene ? sceneModel.activeScene.jointColor : "white"
                        border.color: palette.mid
                        border.width: 1
                    }

                    Button {
                        Layout.row: 0
                        Layout.column: 2
                        Layout.fillWidth: true
                        text: qsTr("Choose color…")
                        enabled: sceneModel.activeScene
                        onClicked: root.openJointColorDialog()
                    }

                    Label {
                        text: qsTr("Bone")
                        Layout.row: 1
                        Layout.column: 0
                        Layout.rowSpan: 2
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: 6
                    }

                    ComboBox {
                        id: boneModeCombo
                        Layout.row: 1
                        Layout.column: 1
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        enabled: sceneModel.activeScene
                        model: [
                            qsTr("Same as joint color"),
                            qsTr("Tone offset from joint"),
                            qsTr("Custom bone color")
                        ]
                        currentIndex: sceneModel.activeScene
                                      ? sceneModel.activeScene.boneColorMode
                                      : root.boneModeToneOffset
                        onActivated: root.setBoneColorMode(currentIndex)
                    }

                    Rectangle {
                        Layout.row: 2
                        Layout.column: 1
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        radius: 2
                        color: sceneModel.activeScene ? sceneModel.activeScene.boneColor : "white"
                        border.color: palette.mid
                        border.width: 1
                    }

                    Button {
                        Layout.row: 2
                        Layout.column: 2
                        Layout.fillWidth: true
                        text: qsTr("Choose color…")
                        enabled: sceneModel.activeScene
                                  && sceneModel.activeScene.boneColorMode === root.boneModeCustom
                        onClicked: root.openBoneColorDialog()
                    }

                    Label {
                        Layout.row: 3
                        Layout.column: 1
                        text: qsTr("Tone:")
                        enabled: sceneModel.activeScene
                                 && sceneModel.activeScene.boneColorMode === root.boneModeToneOffset
                    }

                    ToneColorBar {
                        id: toneSlider
                        Layout.row: 3
                        Layout.column: 2
                        Layout.fillWidth: true
                        jointColor: sceneModel.activeScene ? sceneModel.activeScene.jointColor : "white"
                        value: sceneModel.activeScene ? sceneModel.activeScene.boneTone : Bvh3DModel.defaultBoneTone()
                        enabled: sceneModel.activeScene
                                 && sceneModel.activeScene.boneColorMode === root.boneModeToneOffset
                        helpText: qsTr("Same hue as the joint; drag to adjust bone lightness.")
                        onMoved: {
                            if (sceneModel.activeScene) {
                                sceneModel.activeScene.boneTone = value
                            }
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
        }
    }

    Connections {
        target: sceneModel.activeScene
        enabled: sceneModel.activeScene
        function onSceneOffsetChanged() {
            syncOffsetSpinBoxes()
        }
        function onBoneColorModeChanged() {
            boneModeCombo.currentIndex = sceneModel.activeScene.boneColorMode
        }
    }

    function syncOffsetSpinBoxes() {
        if (!sceneModel.activeScene) {
            return
        }
        suppressOffsetWrite = true
        offsetXSpinBox.value = sceneModel.activeScene.sceneOffset.x
        offsetYSpinBox.value = sceneModel.activeScene.sceneOffset.y
        offsetZSpinBox.value = sceneModel.activeScene.sceneOffset.z
        suppressOffsetWrite = false
    }
}
