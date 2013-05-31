import QtQuick 2.0
import Qt3D 2.0
import Qt3D.Shapes 2.0

Item3D {

    id: scene
    property variant offAxisProjectionMatrix: []

    property real aspectRatio: monitorMesh.aspectRatio
    property real aspectFullScreen: monitorMesh.aspectFullScreen
    property real halfScale: monitorMesh.halfScale
    property real halfScaleRatio: monitorMesh.halfScaleRatio

    MonitorMesh {
        id: monitorMesh
    }

    Cube {
        scale: 50
        effect: programCube
        position: Qt.vector3d(10,10,-100)
        cullFaces: "CullBackFaces"
    }

    Cube {
        scale: 80
        effect: programCube
        position: Qt.vector3d(70,-30,-200)
        cullFaces: "CullBackFaces"
    }

    Teapot {
        scale: 20
        effect: programTeapot
        position: Qt.vector3d(60,60,-30)
        cullFaces: "CullBackFaces"
    }

    Shader {
        id: programLayout
        texture: "qrc:/textures/layout"
        offAxisProjectionMatrix: scene.offAxisProjectionMatrix
        material: Material {
            shininess: 128
            ambientColor: "#EEDD00"
            emittedLight: "#00FFFF"

        }
    }

    Shader {
        id: programCube
        texture: "qrc:/textures/hazard"
        offAxisProjectionMatrix: scene.offAxisProjectionMatrix
        material: Material {
            shininess: 120
            ambientColor: "#EEDD00"
            emittedLight: "#00FF00"
        }
    }

    Shader {
        id: programTeapot
        texture: "qrc:/textures/retro"
        offAxisProjectionMatrix:scene.offAxisProjectionMatrix
        material: Material {
            shininess: 120
            ambientColor: "#EEDD00"
            emittedLight: "#00FF00"
        }
    }
}
