import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BvhScene

ColumnLayout {
    id: root

    required property SceneManager sceneManager

    signal removeRequested()

    spacing: 12

    Label {
        Layout.fillWidth: true
        wrapMode: Text.WordWrap
        text: sceneManager.activeScene
              ? qsTr("Remove \"%1\" from the loaded scenes. This frees memory and cannot be undone.")
                .arg(sceneManager.activeScene.displayName)
              : ""
    }

    Button {
        text: qsTr("Remove scene…")
        Layout.fillWidth: true
        enabled: sceneManager.activeIndex >= 0
        onClicked: root.removeRequested()
    }
}
