import QtQuick
import QtQuick.Controls

import BvhScene

Slider {
    id: root

    property color jointColor: "white"
    property string helpText: ""
    property bool tipVisible: false

    from: -100
    to: 100
    stepSize: 5

    implicitHeight: 24

    readonly property color darkColor: Bvh3DModel.colorFromTone(jointColor, from)
    readonly property color lightColor: Bvh3DModel.colorFromTone(jointColor, to)

    HoverHandler {
        id: barHover
        parent: root

        onHoveredChanged: {
            if (hovered && root.enabled && root.helpText.length > 0) {
                tipTimer.start()
            } else {
                tipTimer.stop()
                root.tipVisible = false
            }
        }
    }

    Timer {
        id: tipTimer
        interval: 700
        repeat: false
        onTriggered: root.tipVisible = true
    }

    ToolTip {
        id: tip
        parent: root
        text: root.helpText
        visible: root.tipVisible
    }

    background: Rectangle {
        x: root.leftPadding
        y: root.topPadding + (root.availableHeight - height) / 2
        implicitWidth: 200
        implicitHeight: 10
        width: root.availableWidth
        height: implicitHeight
        radius: 4
        border.color: root.palette.mid
        border.width: 1
        opacity: root.enabled ? 1.0 : 0.45

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: root.darkColor }
            GradientStop { position: 0.5; color: root.jointColor }
            GradientStop { position: 1.0; color: root.lightColor }
        }
    }

    handle: Rectangle {
        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding + (root.availableHeight - height) / 2
        implicitWidth: 10
        implicitHeight: root.availableHeight
        width: implicitWidth
        height: implicitHeight
        radius: 3
        color: root.palette.text
        border.color: root.palette.base
        border.width: 1
    }
}
