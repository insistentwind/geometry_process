#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    glWidget = new GLWidget(this);
    setCentralWidget(glWidget);
    createMenus();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open"), this, &MainWindow::openFile);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open OBJ File"), "", tr("OBJ Files (*.obj)"));

    if (!fileName.isEmpty() && glWidget->loadObject(fileName)) {
        // 发送信号，通知模型已加载
        emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
    }
}

void MainWindow::updateMesh(const std::vector<QVector3D>& vertices,
                          const std::vector<unsigned int>& indices)
{
    // 更新GLWidget中的显示
    glWidget->updateMesh(vertices, indices);
}