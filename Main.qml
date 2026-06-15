import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

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

            sceneModel: sceneManager

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

            Rectangle {
                id: animationController
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? 56 : 0
                Layout.maximumHeight: visible ? 56 : 0
                visible: sceneManager.frameCount > 0
                height: visible ? 56 : 0
                color: palette.base
                border.color: palette.mid
                border.width: 1

                property bool showFrameNumbers: false

                function padLeft(str, width) {
                    var s = str
                    while (s.length < width)
                        s = " " + s
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
                        if (minStr.length < 2)
                            minStr = "0" + minStr
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

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12
                    Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                    Button {
                        id: playButton
                        text: sceneManager.playing ? qsTr("Pause") : qsTr("Play")
                        enabled: sceneManager.frameCount > 1
                        onClicked: sceneManager.toggle()
                    }

                    Button {
                        id: resetButton
                        text: qsTr("Reset")
                        enabled: !sceneManager.playing
                        onClicked: {
                            sceneManager.pause()
                            sceneManager.animationTime = 0
                        }
                    }

                    Slider {
                        id: timeSlider
                        Layout.fillWidth: true
                        Layout.preferredWidth: animationController.width * 0.45
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
                        text: animationController.showFrameNumbers
                              ? animationController.frameDisplayText(
                                    sceneManager.currentFrame,
                                    Math.max(0, sceneManager.frameCount - 1))
                              : animationController.timeDisplayText(
                                    sceneManager.animationTime,
                                    sceneManager.duration)
                    }

                    TextMetrics {
                        id: labelMetrics
                        font: timelineLabel.font
                        text: animationController.showFrameNumbers
                              ? animationController.frameDisplayText(
                                    Math.max(0, sceneManager.frameCount - 1),
                                    Math.max(0, sceneManager.frameCount - 1))
                              : animationController.timeDisplayText(
                                    sceneManager.duration,
                                    sceneManager.duration)
                    }

                    ToolButton {
                        text: animationController.showFrameNumbers ? qsTr("s") : qsTr("#")
                        ToolTip.visible: hovered
                        ToolTip.text: animationController.showFrameNumbers
                                       ? qsTr("Show time")
                                       : qsTr("Show frames")
                        onClicked: animationController.showFrameNumbers = !animationController.showFrameNumbers
                    }
                }
            }
        }
    }

    Timer {
        id: playbackTimer
        interval: 16
        repeat: true
        running: sceneManager.playing
        onTriggered: {
            const dt = interval / 1000.0
            const next = sceneManager.animationTime + dt
            if (next >= sceneManager.duration) {
                sceneManager.animationTime = sceneManager.duration
                sceneManager.playing = false
            } else {
                sceneManager.animationTime = next
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
