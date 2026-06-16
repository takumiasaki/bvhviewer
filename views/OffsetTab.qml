import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BvhScene

ColumnLayout {
    id: root

    required property SceneManager sceneManager

    property bool suppressOffsetWrite: false

    spacing: 8

    function resetToDefault() {
        if (!sceneManager.activeScene) {
            return
        }
        suppressOffsetWrite = true
        sceneManager.activeScene.sceneOffset = Qt.vector3d(0, 0, 0)
        suppressOffsetWrite = false
    }

    function syncSpinBoxes() {
        if (!sceneManager.activeScene) {
            return
        }
        suppressOffsetWrite = true
        offsetXSpinBox.value = sceneManager.activeScene.sceneOffset.x
        offsetYSpinBox.value = sceneManager.activeScene.sceneOffset.y
        offsetZSpinBox.value = sceneManager.activeScene.sceneOffset.z
        suppressOffsetWrite = false
    }

    RowLayout {
        spacing: 8
        Label { text: qsTr("Offset X") }
        SpinBox {
            id: offsetXSpinBox
            from: -1000
            to: 1000
            stepSize: 10
            value: sceneManager.activeScene ? sceneManager.activeScene.sceneOffset.x : 0
            onValueChanged: {
                if (root.suppressOffsetWrite || !sceneManager.activeScene) {
                    return
                }
                const o = sceneManager.activeScene.sceneOffset
                sceneManager.activeScene.sceneOffset = Qt.vector3d(value, o.y, o.z)
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
            value: sceneManager.activeScene ? sceneManager.activeScene.sceneOffset.y : 0
            onValueChanged: {
                if (root.suppressOffsetWrite || !sceneManager.activeScene) {
                    return
                }
                const o = sceneManager.activeScene.sceneOffset
                sceneManager.activeScene.sceneOffset = Qt.vector3d(o.x, value, o.z)
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
            value: sceneManager.activeScene ? sceneManager.activeScene.sceneOffset.z : 0
            onValueChanged: {
                if (root.suppressOffsetWrite || !sceneManager.activeScene) {
                    return
                }
                const o = sceneManager.activeScene.sceneOffset
                sceneManager.activeScene.sceneOffset = Qt.vector3d(o.x, o.y, value)
            }
        }
    }

    Button {
        text: qsTr("Reset to default")
        Layout.fillWidth: true
        onClicked: root.resetToDefault()
    }

    Connections {
        target: sceneManager
        function onActiveIndexChanged() {
            if (sceneManager.activeIndex < 0) {
                return
            }
            root.syncSpinBoxes()
        }
    }

    Connections {
        target: sceneManager.activeScene
        enabled: sceneManager.activeScene
        function onSceneOffsetChanged() {
            root.syncSpinBoxes()
        }
    }
}
