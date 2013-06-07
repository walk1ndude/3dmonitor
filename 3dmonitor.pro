#-------------------------------------------------
#
# Project created by QtCreator 2012-07-19T19:02:20
#
#-------------------------------------------------

QT       += core gui opengl quick qml 3d 3dquick

greaterThan(QT_MAJOR_VERSION, 5)

TARGET = 3dmonitor
TEMPLATE = app

unix {
    QMAKE_CXX=g++
    QMAKE_CXXFLAGS += -std=c++0x
}

SOURCES += src/main.cpp \
           src/mainwindow.cpp \
           src/cameraoutput.cpp \
           src/cameracapture.cpp \
           src/capturedface.cpp

HEADERS  += src/mainwindow.h \
            src/cameraoutput.h \
            src/cameracapture.h \
            src/capturedface.h

unix {
    CONFIG += link_pkgconfig warn_on
    PKGCONFIG += opencv
}

win32 {
    INCLUDEPATH += C:\opencv\build\include\
    LIBS += -L C:\opencv\build\x64\mingw\lib -libopencv_core245 \
                                         -libopencv_highgui245 \
                                         -libopencv_imgproc245 \
                                         -libopencv_video245 \
                                         -libopencv_objdetect245
}

RESOURCES += res/resources.qrc

OTHER_FILES += \
    res/qml/main.qml \
    res/qml/Slider.qml \
    res/qml/MeshUnit.qml \
    res/qml/MonitorMesh.qml \
    res/qml/Shader.qml \
    res/qml/Scene.qml \
    res/qml/Monitor3D.qml
