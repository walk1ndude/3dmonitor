#include <QtGui/QGuiApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);
    MainWindow *window = new MainWindow;

    // if window is not valid terminate app
    if (window->isValid()){
        window->show();
        return a.exec();
    }
}
