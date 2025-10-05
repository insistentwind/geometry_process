#include <QApplication>
#include <mainwindow.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);
    window.setWindowTitle("Work 2"); // 设置不同的窗口标题以区分
    window.show();
    return app.exec();
}