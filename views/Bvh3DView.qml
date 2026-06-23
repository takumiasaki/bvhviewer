import QtQuick
import QtQuick3D
import QtQuick3D.Helpers

import BvhScene

Item {
    id: root
    property SceneManager sceneManager
    readonly property ViewportSettings viewportSettings: sceneManager ? sceneManager.viewportSettings : null

    // Built-in #Sphere / #Cylinder meshes are 100 units along each axis.
    readonly property real builtinMeshSize: 100
    readonly property real jointDiameter: 4
    readonly property real endSiteDiameter: 3
    readonly property real boneDiameter: 2

    View3D {
        id: view3d
        anchors.fill: parent
        environment: SceneEnvironment {
            clearColor: "#101010"
        }

        Node {
            id: cameraOrigin
            position: Qt.vector3d(0, 200, 0)
            eulerRotation: Qt.vector3d(-20, 0, 0)

            PerspectiveCamera {
                id: perspectiveCamera
                position: Qt.vector3d(0, 0, 1300)
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
