import QtQuick 2.0
import widgets 1.0
import Qt3D 2.0
import Qt3D.Shapes 2.0

Rectangle {

    id: mainapp
    width: 700
    height: 480
    color: "lightgray"
    focus: true

    property size fullscreenSize: Qt.size(fullHeight * aspectRatio,fullHeight)

    Monitor3D {
        // 3D monitor itself
        id: monitor3D
        objectName: "Monitor3D"
        size: Qt.size(480,480)
        normalSize: Qt.size(480,480)
        frameSize: Qt.size(cameraOutput.width,cameraOutput.height)
    }

    CameraOutput {
        // its where we view frames from the camera
        id: cameraOutput
        objectName: "CameraOutput"
        size: Qt.size(320,240)
        renderTarget: "FramebufferObject"
        anchors.left: monitor3D.right
    }

    signal sminChanged(int value)
    signal vminChanged(int value)
    signal vmaxChanged(int value)

    Grid {
        // here we can change value and saturation for CAMShift
        id: paramGrid
        columns: 2
        rows: 3
        spacing: 15

        Text {
            id: smin
            text: qsTr("smin")
        }

        Slider {
            id: slsMin
            objectName: qsTr("min saturation: ")
            width: cameraOutput.width - (Math.max(smin.width,vmin.width,vmax.width) + 20)
            value: 65
            onValueChanged: {
                updatePos();
                sminChanged(slsMin.value);
            }
        }

        Text {
            id: vmin
            text: qsTr("vmin")
        }

        Slider {
            id: slvMin
            objectName: qsTr("min value: ")
            width: cameraOutput.width - (Math.max(smin.width,vmin.width,vmax.width) + 20)
            value: 55
            onValueChanged: {
                updatePos();
                vminChanged(slvMin.value);
            }
        }

        Text {
            id: vmax
            text: qsTr("vmax")
        }
        Slider {
            id: slvMax
            objectName: qsTr("max value: ")
            width: cameraOutput.width - (Math.max(smin.width,vmin.width,vmax.width) + 20)
            value: 130
            onValueChanged: {
                updatePos();
                vmaxChanged(slvMax.value);
            }
        }

        anchors.left: monitor3D.right
        anchors.top: cameraOutput.bottom
        anchors.topMargin: 15
    }

    Grid {
        id: keys
        columns: 1
        rows: 8

        Text {
            text: qsTr("Key B: get BackProject")
        }

        Text {
            text: qsTr("Key H: get HSV image")
        }

        Text {
            text: qsTr("Key K: enable Kalman filter")
        }

        Text {
            text: qsTr("Key R: restart tracking")
        }

        Text {
            text: qsTr("Key F: go fullscreen")
        }
        Text {
            text: qsTr("Key Up: timer +10 ms")
        }
        Text {
            text: qsTr("Key Down: timer -10 ms")
        }
        Text {
            text: qsTr("Current timer's interval: ") + trTimer.interval + qsTr(" ms")
        }

        anchors.top: paramGrid.bottom
        anchors.left: monitor3D.right
    }

    signal signalRecapture()

    Timer {
        // timer to grab new frame
        id: trTimer
        repeat: true
        running: true
        // because 1000 ms / 30 fps
        interval: 34
        onTriggered: signalRecapture()
    }

    signal signalBpressed()
    signal signalHpressed()
    signal signalKpressed()
    signal signalRpressed()

    Keys.onEscapePressed: Qt.quit()
    Keys.onPressed: {
        switch (event.key){
            case Qt.Key_B:
                signalBpressed()
                break
            case Qt.Key_H:
                signalHpressed()
                break
            case Qt.Key_K:
                signalKpressed()
                break
            case Qt.Key_R:
                signalRpressed()
                break
            case Qt.Key_F:
                goFullScreen()
                break
            case Qt.Key_Up:
                trTimer.interval += 10
                break
            case Qt.Key_Down:
                if (trTimer.interval > 10) {
                    trTimer.interval -= 10
                }
        }
    }

    signal windowStateChanged()

    function goFullScreen() {

        // change window state of the monitor, make it normal size or make it fullscreen

        windowStateChanged()

        if (monitor3D.size == mainapp.fullscreenSize) {
            monitor3D.setSize(monitor3D.normalSize)
        }
        else {
            monitor3D.setSize(mainapp.fullscreenSize)
        }

        // need to recalculate off axis projection matrix in shader program, because of the aspect ratio
        monitor3D.parallaxEffect()

        // hide element which are not visible in fullscreen
        cameraOutput.visible = !cameraOutput.visible
        paramGrid.visible = !paramGrid.visible
        keys.visible = !keys.visible

    }
}

