import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import BvhScene

Rectangle {
    id: root

    required property SceneManager sceneManager

    property bool playing: false
    property bool showFrameNumbers: false

    visible: sceneManager.frameCount > 0
    Layout.fillWidth: true
    Layout.preferredHeight: visible ? 56 : 0
    Layout.maximumHeight: visible ? 56 : 0
    height: visible ? 56 : 0
    color: palette.base
    border.color: palette.mid
    border.width: 1

    function play() {
        if (sceneManager.frameCount <= 1 || sceneManager.duration <= 0) {
            return
        }
        if (sceneManager.animationTime >= sceneManager.duration) {
            sceneManager.animationTime = 0
        }
        playing = true
    }

    function pause() {
        playing = false
    }

    function toggle() {
        if (playing) {
            pause()
        } else {
            play()
        }
    }

    function padLeft(str, width) {
        var s = str
        while (s.length < width) {
            s = " " + s
        }
        return s
    }

    function formatSecondsPart(seconds) {
        var secStr = seconds.toFixed(2)
        return secStr.length < 5 ? "0" + secStr : secStr
    }

    function formatDuration(seconds) {
        seconds = Math.max(0, seconds)
        var totalMinutes = Math.floor(seconds / 60)
        var secs = seconds - totalMinutes * 60
        var hours = Math.floor(totalMinutes / 60)
        var minutes = totalMinutes % 60

        if (hours > 0) {
            var minStr = minutes.toString()
            if (minStr.length < 2) {
                minStr = "0" + minStr
            }
            return hours + ":" + minStr + ":" + formatSecondsPart(secs)
        }

        return totalMinutes + ":" + formatSecondsPart(secs)
    }

    function timeDisplayText(current, total) {
        var totalStr = formatDuration(total)
        var currentStr = formatDuration(current)
        return padLeft(currentStr, totalStr.length) + " / " + totalStr
    }

    function frameDisplayText(current, total) {
        var totalStr = total.toString()
        var currentStr = current.toString()
        return qsTr("Frame %1 / %2").arg(padLeft(currentStr, totalStr.length)).arg(totalStr)
    }

    Timer {
        id: playbackTimer
        interval: 16
        repeat: true
        running: root.playing
        onTriggered: {
            const dt = interval / 1000.0
            const next = sceneManager.animationTime + dt
            if (next >= sceneManager.duration) {
                sceneManager.animationTime = sceneManager.duration
                root.playing = false
            } else {
                sceneManager.animationTime = next
            }
        }
    }

    Connections {
        target: sceneManager
        function onFrameCountChanged() {
            if (sceneManager.frameCount <= 0) {
                root.playing = false
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

        Button {
            text: root.playing ? qsTr("Pause") : qsTr("Play")
            enabled: sceneManager.frameCount > 1
            onClicked: root.toggle()
        }

        Button {
            text: qsTr("Reset")
            enabled: !root.playing
            onClicked: {
                root.pause()
                sceneManager.animationTime = 0
            }
        }

        Slider {
            id: timeSlider
            Layout.fillWidth: true
            Layout.preferredWidth: root.width * 0.45
            from: 0
            to: sceneManager.duration > 0 ? sceneManager.duration : 0
            stepSize: sceneManager.frameTime > 0 ? sceneManager.frameTime : 0.001
            value: sceneManager.animationTime
            live: true
            onMoved: sceneManager.animationTime = value
        }

        Label {
            id: timelineLabel
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
            font.family: "Consolas, Courier New, monospace"
            Layout.minimumWidth: labelMetrics.width
            text: root.showFrameNumbers
                  ? root.frameDisplayText(
                        sceneManager.currentFrame,
                        Math.max(0, sceneManager.frameCount - 1))
                  : root.timeDisplayText(
                        sceneManager.animationTime,
                        sceneManager.duration)
        }

        TextMetrics {
            id: labelMetrics
            font: timelineLabel.font
            text: root.showFrameNumbers
                  ? root.frameDisplayText(
                        Math.max(0, sceneManager.frameCount - 1),
                        Math.max(0, sceneManager.frameCount - 1))
                  : root.timeDisplayText(
                        sceneManager.duration,
                        sceneManager.duration)
        }

        ToolButton {
            text: root.showFrameNumbers ? qsTr("s") : qsTr("#")
            ToolTip.visible: hovered
            ToolTip.text: root.showFrameNumbers ? qsTr("Show time") : qsTr("Show frames")
            onClicked: root.showFrameNumbers = !root.showFrameNumbers
        }
    }
}
