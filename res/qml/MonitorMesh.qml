import QtQuick 2.0
import Qt3D 2.0
import Qt3D.Shapes 2.0

Item3D {
    id: monitorMesh
    // monitorMesh.depth / (monitorMesh.height / 2)
    property real aspectRatio: 3.0
    property real aspectFullScreen: 1.0
    // monitorMesh.height / 2
    property real halfScale: meshBottom.halfScale
    // MonitorMesh.width / 2
    property real halfScaleRatio: meshBottom.halfScale * aspectFullScreen

    /*
                meshTop
      meshLeft  meshBack  meshRight
                meshBottom
      */

    MeshUnit {
        id: meshBottom
        position: Qt.vector3d(0, - halfScale, - aspectRatio * halfScale)
        transform: [
            Rotation3D { angle: 180; axis: Qt.vector3d(0, 1, 0) },
            Scale3D { scale: Qt.vector3d(1.0 * aspectFullScreen, 1.0, 1.0 * aspectRatio) }
        ]
    }

    MeshUnit {
        id: meshLeft
        position: Qt.vector3d(- halfScale * aspectFullScreen, 0, - aspectRatio * halfScale)
        transform: [
            Rotation3D { angle: 90; axis: Qt.vector3d(1, 0, 0) },
            Rotation3D { angle: 90; axis: Qt.vector3d(0, 1, 0) },
            Scale3D { scale: Qt.vector3d(1.0 * aspectFullScreen, 1.0, 1.0 * aspectRatio) },
            Rotation3D {
                id: meshLeftRotationY
                axis: Qt.vector3d(0, 1, 0)
                origin: Qt.vector3d(0, 0, meshLeft.halfScale * aspectRatio)
            }
        ]

    }

    MeshUnit {
        id: meshTop
        position: Qt.vector3d(0, halfScale, - aspectRatio * halfScale)
        transform: [
            Rotation3D { angle: 180; axis: Qt.vector3d(1, 0, 0) },
            Scale3D { scale: Qt.vector3d(1.0 * aspectFullScreen, 1.0, 1.0 * aspectRatio) }
        ]
    }

    MeshUnit {
        id: meshRight
        position: Qt.vector3d(halfScale * aspectFullScreen, 0, - aspectRatio * halfScale)
        transform: [
            Rotation3D { angle: 180; axis: Qt.vector3d(0, 1, 0) },
            Rotation3D { angle: 90; axis: Qt.vector3d(0, 0, 1) },
            Scale3D { scale: Qt.vector3d(1.0 * aspectFullScreen, 1.0, 1.0 * aspectRatio) },
            Rotation3D {
                id: meshRightRotationY
                axis: Qt.vector3d(0, 1, 0)
                origin: Qt.vector3d(0, 0, meshRight.halfScale * aspectRatio)
            }
         ]

        }

    MeshUnit {
        id: meshBack
        position: Qt.vector3d(0, 0, - 2 * halfScale * aspectRatio)
        transform: [
            Rotation3D { angle: 90; axis: Qt.vector3d(1, 0, 0) },
            Scale3D { scale: Qt.vector3d(1.0 * aspectFullScreen, 1.0, 1.0 * aspectRatio) }
        ]
    }
}
