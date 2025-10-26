#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent) {
    // 创建UI控件
    glWidget         = new GLWidget(this);
    restoreButton    = new QPushButton(tr("recover model"), this);
    processButton    = new QPushButton(tr("denoise"), this);
    togglePointsButton  = new QPushButton(tr("hide points"), this);
    colorModeButton     = new QPushButton(tr("mode: points"), this);
    filledFaceButton = new QPushButton(tr("show filled"), this);
    
    // ==================== ARAP 工具按钮和下拉菜单 ====================
    arapToolButton = new QToolButton(this);
  arapToolButton->setText(tr("ARAP"));
    arapToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    arapToolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  
    // 创建下拉菜单
    arapMenu = new QMenu(this);
    
    // 菜单项 1：进入/退出 ARAP
    arapEnterExitAction = arapMenu->addAction(tr("Enter ARAP"));
    connect(arapEnterExitAction, &QAction::triggered, this, &MainWindow::toggleArapMode);
    
    arapMenu->addSeparator();
    
    // 菜单项 2：选择 Fixed 模式
    arapSelectFixedAction = arapMenu->addAction(tr("Select: Fixed"));
    arapSelectFixedAction->setEnabled(false);
    connect(arapSelectFixedAction, &QAction::triggered, this, &MainWindow::setArapModeFixed);
    
    // 菜单项 3：选择 Handle 模式
    arapSelectHandleAction = arapMenu->addAction(tr("Select: Handle"));
    arapSelectHandleAction->setEnabled(false);
    connect(arapSelectHandleAction, &QAction::triggered, this, &MainWindow::setArapModeHandle);
    
    arapMenu->addSeparator();
 
    // 菜单项 4：清除固定点
  arapClearFixedAction = arapMenu->addAction(tr("Clear Fixed"));
    arapClearFixedAction->setEnabled(false);
    connect(arapClearFixedAction, &QAction::triggered, this, &MainWindow::clearArapFixed);
    
    // 设置工具按钮的菜单和默认操作
  arapToolButton->setMenu(arapMenu);
    arapToolButton->setDefaultAction(arapEnterExitAction);
    
    // 按钮布局
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addWidget(togglePointsButton);
    buttonLayout->addWidget(colorModeButton);
    buttonLayout->addWidget(filledFaceButton);
    buttonLayout->addWidget(arapToolButton);
    buttonLayout->addStretch();

    // 主布局
    auto *central    = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // 信号槽连接
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

/* ==================== ARAP交互槽函数实现 ==================== */

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
    
    // 更新主按钮文本
    arapToolButton->setText(arapActive ? tr("ARAP ?") : tr("ARAP"));
    
    // 更新菜单项文本
    arapEnterExitAction->setText(arapActive ? tr("Exit ARAP") : tr("Enter ARAP"));
  
    // 根据ARAP模式状态启用/禁用子菜单项
    arapSelectFixedAction->setEnabled(arapActive);
    arapSelectHandleAction->setEnabled(arapActive);
    arapClearFixedAction->setEnabled(arapActive);
    
    // 更新选择模式的勾选状态
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