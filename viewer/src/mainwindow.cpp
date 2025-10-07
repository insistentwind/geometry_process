#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // 子控件创建
    glWidget      = new GLWidget(this);
    restoreButton = new QPushButton(tr("recover model"), this);
    processButton = new QPushButton(tr("denoise"), this);

    // 顶部按钮条
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addStretch();

    // 主布局
    auto *central   = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // 连接信号槽
    connect(restoreButton, &QPushButton::clicked,
            this, &MainWindow::restoreModel);
    connect(processButton, &QPushButton::clicked,
            this, &MainWindow::requestProcess);

    createMenus();
}

MainWindow::~MainWindow() = default;

void MainWindow::createMenus() {
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open"), this, &MainWindow::openFile);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), "", tr("OBJ Files (*.obj)"));
    if (!fileName.isEmpty() && glWidget->loadObject(fileName)) {
        originalVertices = glWidget->getVertices();
        originalIndices  = glWidget->getIndices();
        glWidget->clearMSTEdges();   // 打开新文件时清除旧高亮
        emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
    }
}

void MainWindow::restoreModel() {
    if (!originalVertices.empty()) {
        glWidget->updateMesh(originalVertices, originalIndices);
        glWidget->clearMSTEdges(); // 清除 MST 高亮
    }
}

void MainWindow::requestProcess() {
    emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
}

void MainWindow::updateMesh(const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices) {
    glWidget->updateMesh(vertices, indices);
}

void MainWindow::updateMeshWithColors(const std::vector<QVector3D>& vertices,
                                      const std::vector<unsigned int>& indices,
                                      const std::vector<QVector3D>& colors) {
    glWidget->updateMeshWithColors(vertices, indices, colors);
}