import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BvhViewer
import BvhScene

ColumnLayout {
    id: root

    required property SceneManager sceneManager

    readonly property int boneModeSameAsJoint: 0
    readonly property int boneModeToneOffset: 1
    readonly property int boneModeCustom: 2

    signal chooseJointColor()
    signal chooseBoneColor()

    spacing: 8

    function resetToDefault() {
        if (sceneManager.activeIndex < 0) {
            return
        }
        sceneManager.resetSkeletonColors(sceneManager.activeIndex)
    }

    function setBoneColorMode(mode) {
        if (!sceneManager.activeScene) {
            return
        }
        sceneManager.activeScene.boneColorMode = mode
    }

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

        ColorSwatch {
            Layout.row: 0
            Layout.column: 1
            color: sceneManager.activeScene ? sceneManager.activeScene.jointColor : "white"
        }

        Button {
            Layout.row: 0
            Layout.column: 2
            Layout.fillWidth: true
            text: qsTr("Choose color…")
            enabled: sceneManager.activeScene
            onClicked: root.chooseJointColor()
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
            enabled: sceneManager.activeScene
            model: [
                qsTr("Same as joint color"),
                qsTr("Tone offset from joint"),
                qsTr("Custom bone color")
            ]
            currentIndex: sceneManager.activeScene
                          ? sceneManager.activeScene.boneColorMode
                          : root.boneModeToneOffset
            onActivated: root.setBoneColorMode(currentIndex)
        }

        ColorSwatch {
            Layout.row: 2
            Layout.column: 1
            color: sceneManager.activeScene ? sceneManager.activeScene.boneColor : "white"
        }

        Button {
            Layout.row: 2
            Layout.column: 2
            Layout.fillWidth: true
            text: qsTr("Choose color…")
            enabled: sceneManager.activeScene
                      && sceneManager.activeScene.boneColorMode === root.boneModeCustom
            onClicked: root.chooseBoneColor()
        }

        Label {
            Layout.row: 3
            Layout.column: 1
            text: qsTr("Tone:")
            enabled: sceneManager.activeScene
                     && sceneManager.activeScene.boneColorMode === root.boneModeToneOffset
        }

        ToneColorBar {
            id: toneSlider
            Layout.row: 3
            Layout.column: 2
            Layout.fillWidth: true
            jointColor: sceneManager.activeScene ? sceneManager.activeScene.jointColor : "white"
            value: sceneManager.activeScene ? sceneManager.activeScene.boneTone : Bvh3DModel.defaultBoneTone()
            enabled: sceneManager.activeScene
                     && sceneManager.activeScene.boneColorMode === root.boneModeToneOffset
            helpText: qsTr("Same hue as the joint; drag to adjust bone lightness.")
            onMoved: {
                if (sceneManager.activeScene) {
                    sceneManager.activeScene.boneTone = value
                }
            }
        }
    }

    Button {
        text: qsTr("Reset to default")
        Layout.fillWidth: true
        enabled: sceneManager.activeIndex >= 0
        onClicked: root.resetToDefault()
    }

    Connections {
        target: sceneManager.activeScene
        enabled: sceneManager.activeScene
        function onBoneColorModeChanged() {
            boneModeCombo.currentIndex = sceneManager.activeScene.boneColorMode
        }
    }
}
