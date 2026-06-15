import QtQuick
import QtQuick.Controls

Item {
    id: root

    property color jointColor: "white"
    property color boneColor: "white"

    signal clicked()

    implicitWidth: 24
    implicitHeight: 24
    width: implicitWidth
    height: implicitHeight

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: palette.mid
        border.width: 1
        radius: 2
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 1

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.clearRect(0, 0, width, height)

            ctx.fillStyle = root.jointColor
            ctx.beginPath()
            ctx.moveTo(0, 0)
            ctx.lineTo(width, 0)
            ctx.lineTo(0, height)
            ctx.closePath()
            ctx.fill()

            ctx.fillStyle = root.boneColor
            ctx.beginPath()
            ctx.moveTo(width, 0)
            ctx.lineTo(width, height)
            ctx.lineTo(0, height)
            ctx.closePath()
            ctx.fill()
        }
    }

    Connections {
        target: root
        function onJointColorChanged() { canvas.requestPaint() }
        function onBoneColorChanged() { canvas.requestPaint() }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    ToolTip.visible: mouseArea.containsMouse
    ToolTip.text: qsTr("Joint (top-left) / Bone (bottom-right). Click to edit colors.")
}
