#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QThread>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickWindow>

#include "cameracapture.h"
#include "cameraoutput.h"

#define X_WINDOW 100
#define Y_WINDOW 100
#define WIDTH_WINDOW 800
#define HEIGHT_WINDOW 480

class MainWindow : public QQuickView
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWindow *parent = nullptr);
    ~MainWindow();
    bool isValid();

private:
    CameraCapture* CVCameraCapture;
    QThread* CVThread;
    qreal aspectRatio, fullHeight;
    bool isValidWindow;

    void registerTypes();
    void loadQML();
    void makeConnections(CameraCapture *CVCameraCapture);
    QSize getScreen0Coordinates();
    void setProperties();
    void initialGeometry();
    void initialFormat();

public slots:
    void changeWindowState();
};

#endif // MAINWINDOW_H
