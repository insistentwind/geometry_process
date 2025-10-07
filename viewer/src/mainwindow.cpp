#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    glWidget = new GLWidget(this);
    restoreButton = new QPushButton(tr("recover model"), this);
    processButton = new QPushButton(tr("denoise"), this);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addStretch();

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreModel);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::requestProcess);

    createMenus();
}

MainWindow::~MainWindow() {}

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
        originalIndices = glWidget->getIndices();
        glWidget->clearMSTEdges();
        emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
    }
}

void MainWindow::restoreModel() {
    if (!originalVertices.empty()) {
        glWidget->updateMesh(originalVertices, originalIndices);
        glWidget->clearMSTEdges(); // Çå³ýMST¸ßÁÁ
    }
}

void MainWindow::requestProcess() {
    emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
}

void MainWindow::updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices) {
    glWidget->updateMesh(vertices, indices);
}

void MainWindow::updateMeshWithColors(const std::vector<QVector3D>& vertices,
                                      const std::vector<unsigned int>& indices,
                                      const std::vector<QVector3D>& colors) {
    glWidget->updateMeshWithColors(vertices, indices, colors);
}