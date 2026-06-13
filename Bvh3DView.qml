import QtQuick
import QtQuick3D
import QtQuick3D.Helpers
import BvhScene 1.0

Item {
    id: root
    property SceneManager sceneManager

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
                model: sceneManager ? sceneManager.count : 0

                Node {
                    required property int index
                    property BvhSkeletonItem skeleton: sceneManager ? sceneManager.skeletonAt(index) : null

                    visible: skeleton ? skeleton.visible : false
                    position: skeleton ? skeleton.sceneOffset : Qt.vector3d(0, 0, 0)

                    Repeater3D {
                        model: skeleton ? skeleton.jointModel : null

                        Model {
                            required property vector3d position
                            required property bool isEndSite

                            source: "#Sphere"
                            position: position
                            scale: Qt.vector3d(isEndSite ? 0.3 : 0.5, isEndSite ? 0.3 : 0.5, isEndSite ? 0.3 : 0.5)
                            materials: [
                                PrincipledMaterial {
                                    baseColor: skeleton ? skeleton.color : "#ffffff"
                                    roughness: 0.3
                                    metalness: 0.1
                                }
                            ]
                        }
                    }

                    Repeater3D {
                        model: skeleton ? skeleton.boneModel : null

                        Model {
                            required property vector3d position
                            required property quaternion rotation
                            required property real length

                            source: "#Cylinder"
                            position: position
                            rotation: rotation
                            scale: Qt.vector3d(0.2, length > 0 ? length : 0.01, 0.2)
                            materials: [
                                PrincipledMaterial {
                                    baseColor: skeleton ? skeleton.color : "#ffffff"
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
