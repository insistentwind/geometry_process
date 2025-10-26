#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) {
    // ����UI�ؼ�
    glWidget         = new GLWidget(this);
    restoreButton    = new QPushButton(tr("recover model"), this);
    processButton    = new QPushButton(tr("denoise"), this);
    togglePointsButton  = new QPushButton(tr("hide points"), this);
    colorModeButton     = new QPushButton(tr("mode: points"), this);
    filledFaceButton = new QPushButton(tr("show filled"), this);
    
    // ==================== ARAP ���߰�ť�������˵� ====================
    arapToolButton = new QToolButton(this);
  arapToolButton->setText(tr("ARAP"));
    arapToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    arapToolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  
    // ���������˵�
    arapMenu = new QMenu(this);
    
    // �˵��� 1������/�˳� ARAP
    arapEnterExitAction = arapMenu->addAction(tr("Enter ARAP"));
    connect(arapEnterExitAction, &QAction::triggered, this, &MainWindow::toggleArapMode);
    
    arapMenu->addSeparator();
    
    // �˵��� 2��ѡ�� Fixed ģʽ
    arapSelectFixedAction = arapMenu->addAction(tr("Select: Fixed"));
    arapSelectFixedAction->setEnabled(false);
    connect(arapSelectFixedAction, &QAction::triggered, this, &MainWindow::setArapModeFixed);
    
    // �˵��� 3��ѡ�� Handle ģʽ
    arapSelectHandleAction = arapMenu->addAction(tr("Select: Handle"));
    arapSelectHandleAction->setEnabled(false);
    connect(arapSelectHandleAction, &QAction::triggered, this, &MainWindow::setArapModeHandle);
    
    arapMenu->addSeparator();
 
    // �˵��� 4������̶���
  arapClearFixedAction = arapMenu->addAction(tr("Clear Fixed"));
    arapClearFixedAction->setEnabled(false);
    connect(arapClearFixedAction, &QAction::triggered, this, &MainWindow::clearArapFixed);
    
    // ���ù��߰�ť�Ĳ˵���Ĭ�ϲ���
  arapToolButton->setMenu(arapMenu);
    arapToolButton->setDefaultAction(arapEnterExitAction);
    
    // ��ť����
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addWidget(togglePointsButton);
    buttonLayout->addWidget(colorModeButton);
    buttonLayout->addWidget(filledFaceButton);
    buttonLayout->addWidget(arapToolButton);
    buttonLayout->addStretch();

    // ������
    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // �źŲ�����
    connect(restoreButton, &QPushButton::clicked, this, &MainWindow::restoreModel);
    connect(processButton, &QPushButton::clicked, this, &MainWindow::requestProcess);
    connect(togglePointsButton, &QPushButton::clicked, this, &MainWindow::togglePoints);
    connect(colorModeButton, &QPushButton::clicked, this, &MainWindow::cycleColorMode);
    connect(filledFaceButton, &QPushButton::clicked, this, &MainWindow::toggleFilledFaces);

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
      glWidget->clearMSTEdges();
        emit objLoaded(glWidget->getVertices(), glWidget->getIndices());
}
}

void MainWindow::restoreModel() {
    if (!originalVertices.empty()) {
        glWidget->updateMesh(originalVertices, originalIndices);
   glWidget->clearMSTEdges();
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

/* ==================== ARAP�����ۺ���ʵ�� ==================== */

void MainWindow::toggleArapMode() {
    glWidget->toggleArapMode();
    updateArapButtonText();
}

void MainWindow::clearArapFixed() {
    glWidget->clearArapFixedVertices();
}

void MainWindow::setArapModeFixed() {
    glWidget->setArapSelectionMode(GLWidget::ArapSelectionMode::SelectFixed);
    updateArapButtonText();
}

void MainWindow::setArapModeHandle() {
    glWidget->setArapSelectionMode(GLWidget::ArapSelectionMode::SelectHandle);
    updateArapButtonText();
}

void MainWindow::updateArapButtonText() {
    bool arapActive = glWidget->isArapActive();
    
    // ��������ť�ı�
    arapToolButton->setText(arapActive ? tr("ARAP ?") : tr("ARAP"));
    
    // ���²˵����ı�
    arapEnterExitAction->setText(arapActive ? tr("Exit ARAP") : tr("Enter ARAP"));
  
    // ����ARAPģʽ״̬����/�����Ӳ˵���
    arapSelectFixedAction->setEnabled(arapActive);
    arapSelectHandleAction->setEnabled(arapActive);
    arapClearFixedAction->setEnabled(arapActive);
    
    // ����ѡ��ģʽ�Ĺ�ѡ״̬
    if (arapActive) {
        auto mode = glWidget->getArapSelectionMode();
        arapSelectFixedAction->setText(mode == GLWidget::ArapSelectionMode::SelectFixed 
  ? tr("? Select: Fixed") 
            : tr("Select: Fixed"));
 arapSelectHandleAction->setText(mode == GLWidget::ArapSelectionMode::SelectHandle 
        ? tr("? Select: Handle") 
    : tr("Select: Handle"));
    } else {
        arapSelectFixedAction->setText(tr("Select: Fixed"));
 arapSelectHandleAction->setText(tr("Select: Handle"));
    }
}