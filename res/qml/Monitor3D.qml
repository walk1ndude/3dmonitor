import QtQuick 2.0
import Qt3D 2.0
import Qt3D.Shapes 2.0

Viewport {

    id: viewport3D

    property size size: Qt.size(480,480)
    property size normalSize: Qt.size(480,480)
    width: size.width
    height: size.height

    property size frameSize: Qt.size(cameraOutput.size.width,cameraOutput.size.height)
    property real ratioX: ratioZ * 2 * scene3D.halfScaleRatio / frameSize.width
    property real ratioY: ratioZ * 2 * scene3D.halfScale / frameSize.height
    property real ratioZ: 1 // some ratio depending on distance to the viewer, do not know how to use for now

    property int faceX: 0
    property int faceY: 0

    property real aspectFullScreen: scene3D.aspectFullScreen

    camera: Camera {
        id: cameraGL
        eye: Qt.vector3d(0, 0, cameraGL.nearPlane)
        upVector: Qt.vector3d(0, 1, 0)
        center: Qt.vector3d(0, 0, 0)
        fieldOfView: 60
        nearPlane: scene3D.halfScaleRatio / Math.tan(fieldOfView * Math.PI / 360)
        farPlane: nearPlane + 2 * scene3D.aspectRatio * scene3D.halfScale + 1
    }

    light: Light {
        id: lightGL
        position:  Qt.vector3d(0, 0, cameraGL.nearPlane)
        spotDirection: Qt.vector3d(0,0,scene3D.aspectRatio * scene3D.halfScale)
    }

    Scene {
        id: scene3D
        offAxisProjectionMatrix: getFrustrum()
    }

    function redraw(faceXv,faceYv) {

        // recalculate head position, where it will be on the scene
        faceX = (faceXv - frameSize.width / 2) * ratioX
        faceY = (faceYv - frameSize.height / 2) * ratioY
        parallaxEffect()

    }

    function getFrustrum(){

        // get new off axis projection, so we need to calculate new frustrum

        var rightMinusLeft = 2 * scene3D.halfScaleRatio
        var rightPlusLeft = - 2 * faceX
        var topMinusBottom = 2 * scene3D.halfScale
        var topPlusBottom = 2 * faceY
        var farMinusNear = cameraGL.farPlane - cameraGL.nearPlane
        var farPlusNear = cameraGL.farPlane + cameraGL.nearPlane
        var nearPlaneMul2 = 2 * cameraGL.nearPlane


        return [nearPlaneMul2 / rightMinusLeft, 0.0, rightPlusLeft / rightMinusLeft, 0.0,
                0.0, nearPlaneMul2 / topMinusBottom, topPlusBottom / topMinusBottom, 0.0,
                0.0, 0.0, - farPlusNear / farMinusNear, - nearPlaneMul2 * cameraGL.farPlane / farMinusNear,
                0.0, 0.0, -1.0, 0.0]
    }

    function parallaxEffect(){

        // feed new off axis projection matrix to the shader program

        scene3D.offAxisProjectionMatrix = getFrustrum()

        /* reset the camera position, so it view directly in the frustrum,
        so that the front boundaries of the monitor don't go anywhere */
        cameraGL.eye = Qt.vector3d(faceX, - faceY, cameraGL.nearPlane)
        cameraGL.center = Qt.vector3d(faceX , - faceY,0)

        lightGL.position = Qt.vector3d(faceX, - faceY, cameraGL.nearPlane)
        lightGL.spotDirection = Qt.vector3d(faceX, - faceY, scene3D.aspectRatio * scene3D.halfScale)
    }

    function setSize(newSize){
        scene3D.aspectFullScreen = newSize.widht / newSize.height
        size = newSize
    }

}
