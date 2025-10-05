// myWork.cpp: 定义应用程序的入口点。
//

#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.resize(800, 600);
    mainWindow.show();
    return app.exec();
}
