#include <QtCore/QDebug>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>
#include <QtQml/QQmlComponent>
#include <QtQml/QQmlContext>

#include "mainwindow.h"
#include "cameraoutput.h"

MainWindow::MainWindow(QWindow *parent): QQuickView(parent)
{
    isValidWindow = true;
    initialFormat();

    // register CameraOutput
    registerTypes();

    CameraCapture *CVCameraCapture = new CameraCapture;

    if (CVCameraCapture->initializeCV()){

        QThread *CVThread = new QThread;
        CVCameraCapture->moveToThread(CVThread);

        // load qml files
        loadQML();
        // connect some qml elements with camera processing
        makeConnections(CVCameraCapture);

        // start qthread, responsible for camera processing
        CVThread->start();

        // set initial window geometry
        initialGeometry();
    }
    else {

        // if some cascades missing, or no camera, this window is not valid, need to terminate app
        isValidWindow = false;
    }
}

bool MainWindow::isValid(){
    return isValidWindow;
}

void MainWindow::initialFormat(){

    this->setSurfaceType(QSurface::OpenGLSurface);

    QSurfaceFormat f;
    f.setRenderableType(QSurfaceFormat::OpenGL);
    this->setFormat(f);

    this->setClearBeforeRendering(true);
}

void MainWindow::initialGeometry(){
    this->setGeometry(X_WINDOW,Y_WINDOW,WIDTH_WINDOW,HEIGHT_WINDOW);
}

void MainWindow::registerTypes(){
    qmlRegisterType<CameraOutput>("widgets", 1, 0, "CameraOutput");
}

QSize MainWindow::getScreen0Coordinates(){
    // returns size of the first screen, for example QSize(1366,768)
    return QGuiApplication::screens()[0]->virtualGeometry().size();
}

void MainWindow::setProperties(){
    //get screen size for fullscreen
    QSize sizeD = getScreen0Coordinates();
    aspectRatio = qreal(sizeD.width()) / sizeD.height();
    fullHeight = sizeD.height();
    this->rootContext()->setContextProperty("fullHeight", fullHeight);
    // full screen width / full screen height
    this->rootContext()->setContextProperty("aspectRatio", aspectRatio);
}

void MainWindow::loadQML(){
    // resize qml on qquickview
    this->setResizeMode(QQuickView::SizeRootObjectToView);
    // set screen height and aspect ratio for usage in qml
    setProperties();
    // load main qml file
    this->setSource(QUrl("qrc:/qml/main"));
}

void MainWindow::makeConnections(CameraCapture *CVCameraCapture){
    // close on X or esc
    connect((QObject*)this->engine(),SIGNAL(quit()),this,SLOT(close()));

    QObject *root = this->rootObject();

    // connect CVCameraCapture to qml, so every 34ms (30 fps) we get a new frame
    connect(root,SIGNAL(signalRecapture()),CVCameraCapture,SLOT(recapture()));

    connect(CVCameraCapture,SIGNAL(signalRecapture(QImage)),root->findChild<CameraOutput*>("CameraOutput"),SLOT(setCameraimage(QImage)));

    // send head coordinates to monitor
    connect(CVCameraCapture,SIGNAL(signalRedraw(QVariant,QVariant)),root->findChild<QObject*>("Monitor3D"),SLOT(redraw(QVariant,QVariant)));

    // connect keys to some CVCameraCapture parameters: get backproject image, get HSV and so on
    connect(root,SIGNAL(signalBpressed()),CVCameraCapture,SLOT(keyBpressed()),Qt::DirectConnection);
    connect(root,SIGNAL(signalHpressed()),CVCameraCapture,SLOT(keyHpressed()),Qt::DirectConnection);
    connect(root,SIGNAL(signalKpressed()),CVCameraCapture,SLOT(keyKpressed()),Qt::DirectConnection);
    connect(root,SIGNAL(signalRpressed()),CVCameraCapture,SLOT(keyRpressed()),Qt::DirectConnection);

    // connect sliders to some CVCameraCapture parameters: vmax, vmin, smin
    connect(root,SIGNAL(sminChanged(int)),CVCameraCapture,SLOT(sminchanged(int)),Qt::DirectConnection);
    connect(root,SIGNAL(vminChanged(int)),CVCameraCapture,SLOT(vminchanged(int)),Qt::DirectConnection);
    connect(root,SIGNAL(vmaxChanged(int)),CVCameraCapture,SLOT(vmaxchanged(int)),Qt::DirectConnection);

    // to go fullscreen
    connect(root,SIGNAL(windowStateChanged()),this,SLOT(changeWindowState()));
}

MainWindow::~MainWindow(){
    delete CVCameraCapture;
    delete CVThread;
}

void MainWindow::changeWindowState(){
    if (this->windowState() == Qt::WindowFullScreen){
        this->setWindowState(Qt::WindowNoState);
    }
    else {
        this->setWindowState(Qt::WindowFullScreen);
    }
}
