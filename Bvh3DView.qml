import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import BvhScene 1.0

Item {
    id: root
    property SceneManager sceneManager

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
            brightness: 2.0
            eulerRotation: Qt.vector3d(-45, 0, 0)
        }

        Model {
            id: gridPlane
            geometry: PlaneGeometry {
                width: 2000
                height: 2000
            }
            materials: [
                PrincipledMaterial {
                    baseColor: "#383840"
                    roughness: 1.0
                }
            ]
            position: Qt.vector3d(0, 0, 0)
            eulerRotation: Qt.vector3d(-90, 0, 0)
        }

        Node {
            id: skeletonsRoot

            Repeater3D {
                id: skeletonRepeater
                model: sceneManager

                Node {
                    required property int index
                    required property BvhSkeletonItem skeleton

                    visible: skeleton.visible
                    position: skeleton.sceneOffset

                    Repeater3D {
                        model: skeleton.jointModel

                        Model {
                            source: "#Sphere"
                            position: model.position
                            scale: Qt.vector3d(
                                (model.isEndSite ? endSiteDiameter : jointDiameter) / builtinMeshSize,
                                (model.isEndSite ? endSiteDiameter : jointDiameter) / builtinMeshSize,
                                (model.isEndSite ? endSiteDiameter : jointDiameter) / builtinMeshSize)
                            materials: [
                                PrincipledMaterial {
                                    baseColor: skeleton.color
                                    roughness: 0.3
                                    metalness: 0.1
                                }
                            ]
                        }
                    }

                    Repeater3D {
                        model: skeleton.boneModel

                        Model {
                            source: "#Cylinder"
                            position: model.position
                            rotation: model.rotation
                            scale: Qt.vector3d(
                                boneDiameter / builtinMeshSize,
                                (model.length > 0 ? model.length : 0.01) / builtinMeshSize,
                                boneDiameter / builtinMeshSize)
                            materials: [
                                PrincipledMaterial {
                                    baseColor: skeleton.color
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
