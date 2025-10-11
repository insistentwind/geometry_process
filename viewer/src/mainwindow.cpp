#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // �ӿؼ�����
    glWidget            = new GLWidget(this);
    restoreButton       = new QPushButton(tr("recover model"), this);
    processButton       = new QPushButton(tr("denoise"), this);
    togglePointsButton  = new QPushButton(tr("hide points"), this); // ��ʼΪ��ʾ״̬
    colorModeButton     = new QPushButton(tr("mode: points"), this); // ��ʼģʽ
    filledFaceButton    = new QPushButton(tr("show filled"), this); // ����

    // ��ť��
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addWidget(togglePointsButton);
    buttonLayout->addWidget(colorModeButton);
    buttonLayout->addWidget(filledFaceButton);
    buttonLayout->addStretch();

    // ������
    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // �źŲ�
    connect(restoreButton, &QPushButton::clicked,
            this, &MainWindow::restoreModel);
    connect(processButton, &QPushButton::clicked,
            this, &MainWindow::requestProcess);
    connect(togglePointsButton, &QPushButton::clicked,
            this, &MainWindow::togglePoints);
    connect(colorModeButton, &QPushButton::clicked,
            this, &MainWindow::cycleColorMode);
    connect(filledFaceButton, &QPushButton::clicked,
            this, &MainWindow::toggleFilledFaces);

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
        glWidget->clearMSTEdges();   // ���ļ�ʱ����ɸ���
        emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
    }
}

void MainWindow::restoreModel() {
    if (!originalVertices.empty()) {
        glWidget->updateMesh(originalVertices, originalIndices);
        glWidget->clearMSTEdges(); // ��� MST ��
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

void MainWindow::togglePoints() {
    pointsVisible = !pointsVisible;
    glWidget->setShowColoredPoints(pointsVisible);
    togglePointsButton->setText(pointsVisible ? tr("hide points") : tr("show points"));
}

void MainWindow::cycleColorMode() {
    glWidget->cycleColorDisplayMode();
    updateColorModeButtonText();
}

void MainWindow::toggleFilledFaces() {
    glWidget->toggleFilledFaces();
    updateFilledFaceButtonText();
}

void MainWindow::updateColorModeButtonText() {
    switch (glWidget->getColorDisplayMode()) {
        case GLWidget::ColorDisplayMode::PointsOnly:
            colorModeButton->setText(tr("mode: points"));
            break;
        case GLWidget::ColorDisplayMode::Faces:
            colorModeButton->setText(tr("mode: faces"));
            break;
    }
}

void MainWindow::updateFilledFaceButtonText() {
    filledFaceButton->setText(glWidget->isShowingFilledFaces() ? tr("hide filled") : tr("show filled"));
}