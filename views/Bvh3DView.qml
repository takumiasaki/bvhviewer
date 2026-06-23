import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

import BvhScene

Item {
    id: root
    property SceneManager sceneManager
    readonly property ViewportSettings viewportSettings: sceneManager ? sceneManager.viewportSettings : null
    readonly property CameraSettings cameraSettings: sceneManager ? sceneManager.cameraSettings : null

    // Built-in #Sphere / #Cylinder meshes are 100 units along each axis.
    readonly property real builtinMeshSize: 100
    readonly property real jointDiameter: 4
    readonly property real endSiteDiameter: 3
    readonly property real boneDiameter: 2
    readonly property real jointRadius: jointDiameter / 2
    readonly property real endSiteRadius: endSiteDiameter / 2

    readonly property real defaultOriginX: 0
    readonly property real defaultOriginY: 200
    readonly property real defaultOriginZ: 0
    readonly property real defaultPitch: -20
    readonly property real defaultDistance: 1300
    readonly property int frameAnimationDuration: 300

    // Playback follow stabilization (see camera-controls.md §6.3).
    readonly property real followDeadzoneFactor: 0.06
    readonly property real followDeadzoneMinimum: 10
    readonly property real followSmoothingAlpha: 0.15
    readonly property real followSmoothingAlphaMax: 0.4

    property real followDistance: defaultDistance
    property bool framingAnimationActive: false
    property bool pendingFollowDistanceUpdate: false

    readonly property bool autoFollowActive: cameraSettings
            && cameraSettings.mode === CameraSettings.Auto
            && sceneManager
            && sceneManager.playing

    function viewAspectRatio() {
        return view3d.width > 0 ? view3d.width / view3d.height : 1.0
    }

    function vectorLength(dx, dy, dz) {
        return Math.sqrt(dx * dx + dy * dy + dz * dz)
    }

    function resetCamera() {
        framingAnimationActive = false
        originFrameAnimation.stop()
        distanceFrameAnimation.stop()
        cameraOrigin.position = Qt.vector3d(defaultOriginX, defaultOriginY, defaultOriginZ)
        cameraOrigin.eulerRotation = Qt.vector3d(defaultPitch, 0, 0)
        perspectiveCamera.position = Qt.vector3d(0, 0, defaultDistance)
        followDistance = defaultDistance
    }

    function currentFramingDistance() {
        return sceneManager.framingDistance(
                    jointRadius,
                    endSiteRadius,
                    perspectiveCamera.fieldOfView,
                    viewAspectRatio(),
                    defaultPitch)
    }

    function followTarget() {
        if (!sceneManager || !sceneManager.hasFramingTarget(jointRadius, endSiteRadius)) {
            return
        }

        const target = sceneManager.framingCenter(jointRadius, endSiteRadius)
        const current = cameraOrigin.position
        const boundsRadius = sceneManager.framingRadius(jointRadius, endSiteRadius)
        const deadzone = Math.max(followDeadzoneMinimum, boundsRadius * followDeadzoneFactor)

        const requiredDistance = currentFramingDistance()
        let distanceGrew = false
        if (requiredDistance > followDistance) {
            followDistance = requiredDistance
            distanceGrew = true
        }

        const dx = target.x - current.x
        const dy = target.y - current.y
        const dz = target.z - current.z
        const offset = vectorLength(dx, dy, dz)
        if (!distanceGrew && offset < deadzone) {
            perspectiveCamera.position = Qt.vector3d(0, 0, followDistance)
            return
        }

        const alpha = Math.min(
                    followSmoothingAlphaMax,
                    followSmoothingAlpha + offset / Math.max(boundsRadius * 4, 1))
        cameraOrigin.position = Qt.vector3d(
                    current.x + dx * alpha,
                    current.y + dy * alpha,
                    current.z + dz * alpha)
        perspectiveCamera.position = Qt.vector3d(0, 0, followDistance)
    }

    function frameToTarget(animated, instant, updateFollowDistance) {
        if (!sceneManager || !sceneManager.hasFramingTarget(jointRadius, endSiteRadius)) {
            return
        }

        const center = sceneManager.framingCenter(jointRadius, endSiteRadius)
        const distance = currentFramingDistance()

        if (!animated || instant) {
            framingAnimationActive = false
            originFrameAnimation.stop()
            distanceFrameAnimation.stop()
            cameraOrigin.position = center
            perspectiveCamera.position = Qt.vector3d(0, 0, distance)
            if (updateFollowDistance) {
                followDistance = distance
            }
            return
        }

        framingAnimationActive = true
        pendingFollowDistanceUpdate = updateFollowDistance
        originFrameAnimation.to = center
        distanceFrameAnimation.to = distance
        frameAnimationGroup.start()
        return
    }

    function startAutoFollow() {
        frameToTarget(true, false, true)
    }

    ParallelAnimation {
        id: frameAnimationGroup
        running: false

        PropertyAnimation {
            id: originFrameAnimation
            target: cameraOrigin
            property: "position"
            duration: root.frameAnimationDuration
            easing.type: Easing.OutCubic
        }

        PropertyAnimation {
            id: distanceFrameAnimation
            target: perspectiveCamera
            property: "position.z"
            duration: root.frameAnimationDuration
            easing.type: Easing.OutCubic
        }

        onFinished: {
            root.framingAnimationActive = false
            if (root.pendingFollowDistanceUpdate) {
                root.followDistance = perspectiveCamera.position.z
                root.pendingFollowDistanceUpdate = false
            }
        }
    }

    Connections {
        target: sceneManager
        enabled: sceneManager

        function onPlayingChanged() {
            if (sceneManager.playing
                    && cameraSettings
                    && cameraSettings.mode === CameraSettings.Auto) {
                root.startAutoFollow()
            }
        }

        function onFramingTargetChanged() {
            if (root.autoFollowActive) {
                root.followTarget()
            }
        }

        function onFramingRefitRequested() {
            if (root.autoFollowActive) {
                root.frameToTarget(true, false, true)
            }
        }
    }

    Connections {
        target: cameraSettings
        enabled: cameraSettings

        function onFrameNowRequested(instant) {
            root.frameToTarget(true, instant, false)
        }

        function onResetViewRequested() {
            root.resetCamera()
        }

        function onModeChanged() {
            if (cameraSettings.mode === CameraSettings.Auto
                    && sceneManager
                    && sceneManager.playing) {
                root.startAutoFollow()
            }
        }
    }

    View3D {
        id: view3d
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: "#101010"
        }

        Node {
            id: cameraOrigin
            position: Qt.vector3d(root.defaultOriginX, root.defaultOriginY, root.defaultOriginZ)
            eulerRotation: Qt.vector3d(root.defaultPitch, 0, 0)

            PerspectiveCamera {
                id: perspectiveCamera
                position: Qt.vector3d(0, 0, root.defaultDistance)
                fieldOfView: 45
                clipNear: 0.1
                clipFar: 10000
            }
        }

        OrbitCameraController {
            camera: perspectiveCamera
            origin: cameraOrigin
            xSpeed: 0.4
            ySpeed: 0.4
            panEnabled: true
            enabled: !root.framingAnimationActive && !root.autoFollowActive
        }

        DirectionalLight {
            color: "#ffffff"
            ambientColor: Qt.rgba(0.0, 0.0, 0.0, 1.0)
            brightness: 1.8
            eulerRotation: Qt.vector3d(-45, 0, 0)
            castsShadow: root.viewportSettings.floorShadowsEnabled
            shadowMapQuality: Light.ShadowMapQualityVeryHigh
            shadowMapFar: 1200
            pcfFactor: 0.5
        }

        PrincipledMaterial {
            id: groundMaterial
            baseColor: root.viewportSettings.groundColor
            roughness: 1.0
        }

        Model {
            id: groundRectangle
            visible: root.viewportSettings.groundShape === ViewportSettings.Rectangle
            source: "#Cube"
            scale: Qt.vector3d(
                root.viewportSettings.groundSizeX / root.builtinMeshSize,
                root.viewportSettings.groundSizeY / root.builtinMeshSize,
                root.viewportSettings.groundSizeZ / root.builtinMeshSize)
            materials: [groundMaterial]
            position: Qt.vector3d(0, -root.viewportSettings.groundSizeY / 2, 0)
            castsShadows: false
            receivesShadows: true
        }

        Model {
            id: groundEllipse
            visible: root.viewportSettings.groundShape === ViewportSettings.Ellipse
            source: "#Cylinder"
            scale: Qt.vector3d(
                root.viewportSettings.groundSizeX / root.builtinMeshSize,
                root.viewportSettings.groundSizeY / root.builtinMeshSize,
                root.viewportSettings.groundSizeZ / root.builtinMeshSize)
            materials: [groundMaterial]
            position: Qt.vector3d(0, -root.viewportSettings.groundSizeY / 2, 0)
            castsShadows: false
            receivesShadows: true
        }

        Node {
            id: skeletonsRoot

            Repeater3D {
                id: skeletonRepeater
                model: sceneManager

                Node {
                    id: skeletonNode
                    required property int index
                    required property BvhSkeletonItem skeleton

                    visible: skeleton.visible
                    position: skeleton.sceneOffset

                    property color jointColor: skeleton.jointColor
                    property color boneColor: skeleton.boneColor

                    Connections {
                        target: skeletonNode.skeleton
                        function onJointColorChanged() {
                            skeletonNode.jointColor = skeletonNode.skeleton.jointColor
                        }
                        function onBoneColorChanged() {
                            skeletonNode.boneColor = skeletonNode.skeleton.boneColor
                        }
                    }

                    Repeater3D {
                        model: skeletonNode.skeleton.jointModel

                        Model {
                            source: "#Sphere"
                            position: model.position
                            scale: Qt.vector3d(
                                (model.isEndSite ? root.endSiteDiameter : root.jointDiameter) / root.builtinMeshSize,
                                (model.isEndSite ? root.endSiteDiameter : root.jointDiameter) / root.builtinMeshSize,
                                (model.isEndSite ? root.endSiteDiameter : root.jointDiameter) / root.builtinMeshSize)
                            castsShadows: root.viewportSettings.floorShadowsEnabled
                            receivesShadows: false
                            depthBias: 0.15
                            materials: [
                                PrincipledMaterial {
                                    baseColor: skeletonNode.jointColor
                                    roughness: 0.3
                                    metalness: 0.1
                                }
                            ]
                        }
                    }

                    Repeater3D {
                        model: skeletonNode.skeleton.boneModel

                        Model {
                            source: "#Cylinder"
                            position: model.position
                            rotation: model.rotation
                            scale: Qt.vector3d(
                                root.boneDiameter / root.builtinMeshSize,
                                (model.length > 0 ? model.length : 0.01) / root.builtinMeshSize,
                                root.boneDiameter / root.builtinMeshSize)
                            castsShadows: root.viewportSettings.floorShadowsEnabled
                            receivesShadows: false
                            depthBias: 0.15
                            materials: [
                                PrincipledMaterial {
                                    baseColor: skeletonNode.boneColor
                                    roughness: 0.4
                                    metalness: 0.1
                                }
                            ]
                        }
                    }
                }
            }
        }
    }
}
