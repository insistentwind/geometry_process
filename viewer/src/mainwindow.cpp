#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // 创建UI控件
    glWidget            = new GLWidget(this);
    restoreButton       = new QPushButton(tr("recover model"), this);
    processButton       = new QPushButton(tr("denoise"), this);
    togglePointsButton  = new QPushButton(tr("hide points"), this);
    colorModeButton     = new QPushButton(tr("mode: points"), this);
    filledFaceButton = new QPushButton(tr("show filled"), this);
    
    // ==================== ARAP UI控件 ====================
    arapModeButton = new QPushButton(tr("enter ARAP"), this);
    arapClearFixedButton = new QPushButton(tr("clear fixed"), this);
    arapSelectFixedButton = new QPushButton(tr("select: Fixed"), this);  // 新增
    arapSelectHandleButton = new QPushButton(tr("select: Handle"), this); // 新增

  // 按钮布局
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addWidget(togglePointsButton);
    buttonLayout->addWidget(colorModeButton);
    buttonLayout->addWidget(filledFaceButton);
    buttonLayout->addWidget(arapModeButton); // ARAP按钮
    buttonLayout->addWidget(arapSelectFixedButton); // 新增：选择Fixed模式
    buttonLayout->addWidget(arapSelectHandleButton); // 新增：选择Handle模式
    buttonLayout->addWidget(arapClearFixedButton); // 清除固定点按钮
    buttonLayout->addStretch();

    // 主布局
    auto *central    = new QWidget(this);
  auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // 信号槽连接
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
    
    // ==================== ARAP信号槽连接 ====================
    connect(arapModeButton, &QPushButton::clicked,
  this, &MainWindow::toggleArapMode);
    connect(arapSelectFixedButton, &QPushButton::clicked,
            this, &MainWindow::setArapModeFixed);
    connect(arapSelectHandleButton, &QPushButton::clicked,
      this, &MainWindow::setArapModeHandle);
    connect(arapClearFixedButton, &QPushButton::clicked,
     this, &MainWindow::clearArapFixed);

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

/**
 * @brief 切换ARAP模式
 * 
 * 作用：
 * - 调用GLWidget的toggleArapMode()进入/退出ARAP模式
 * - 更新按钮文本显示当前状态
 * 
 * 用户操作流程：
 * 1. 点击"enter ARAP"进入模式
 * 2. 在3D视图中左键点击顶点选择为fixed（可多次点击）
 * 3. 再次左键点击并拖动一个顶点作为handle进行变形
 * 4. 点击"exit ARAP"退出模式
 */
void MainWindow::toggleArapMode() {
    glWidget->toggleArapMode();
    updateArapButtonText();
    updateArapSelectionButtonText(); // 同时更新选择模式按钮
}

/**
 * @brief 清除所有已选择的固定顶点
 * 
 * 作用：
 * - 调用GLWidget的clearArapFixedVertices()
 * - 清空MeshProcessor中所有顶点的fixed标记
 * - 让用户重新选择约束点
 * 
 * 使用场景：
 * - 用户对当前选择的fixed点不满意，想重新选择
 */
void MainWindow::clearArapFixed() {
    glWidget->clearArapFixedVertices();
}

/**
 * @brief 选择Fixed顶点
 * 
 * 作用：
 * - 切换到固定点选择模式
 * - 用户在3D视图中点击顶点时，选中的顶点将被标记为fixed
 * - 支持多次点击，用户可选择多个fixed点
 * 
 * 用户操作流程：
 * 1. 点击"select: Fixed"进入选择固定点模式
 * 2. 在3D视图中左键点击一个或多个顶点进行选择
 * 3. 点击其他按钮或再次点击"select: Fixed"退出模式
 */
void MainWindow::setArapModeFixed() {
    // 切换到固定点选择模式
    glWidget->setArapSelectionMode(GLWidget::ArapSelectionMode::SelectFixed);
    updateArapSelectionButtonText();
}

/**
 * @brief 选择Handle顶点
 * 
 * 作用：
 * - 切换到Handle点选择模式
 * - 用户在3D视图中点击顶点时，选中的顶点将被标记为handle
 * - 只有一个顶点会被标记为handle，多次点击会更新为最新选择
 * 
 * 用户操作流程：
 * 1. 点击"select: Handle"进入选择Handle点模式
 * 2. 在3D视图中左键点击一个顶点进行选择
 * 3. 点击其他按钮或再次点击"select: Handle"退出模式
 */
void MainWindow::setArapModeHandle() {
    // 切换到Handle点选择模式
    glWidget->setArapSelectionMode(GLWidget::ArapSelectionMode::SelectHandle);
    updateArapSelectionButtonText();
}

/**
 * @brief 更新ARAP按钮文本
 * 
 * 根据当前是否处于ARAP模式，显示：
 * - "enter ARAP"：非ARAP模式，点击进入
 * - "exit ARAP"：ARAP模式中，点击退出
 */
void MainWindow::updateArapButtonText() {
    arapModeButton->setText(glWidget->isArapActive() ? tr("exit ARAP") : tr("enter ARAP"));
    
    // 根据ARAP模式状态启用/禁用选择按钮
    bool arapActive = glWidget->isArapActive();
    arapSelectFixedButton->setEnabled(arapActive);
    arapSelectHandleButton->setEnabled(arapActive);
    arapClearFixedButton->setEnabled(arapActive);
}

/**
 * @brief 更新选择模式按钮文本
 * 根据当前选择模式高亮对应按钮
 */
void MainWindow::updateArapSelectionButtonText() {
    auto mode = glWidget->getArapSelectionMode();
  
    if (mode == GLWidget::ArapSelectionMode::SelectFixed) {
        arapSelectFixedButton->setText(tr("? select: Fixed"));
        arapSelectHandleButton->setText(tr("select: Handle"));
    } else if (mode == GLWidget::ArapSelectionMode::SelectHandle) {
   arapSelectFixedButton->setText(tr("select: Fixed"));
   arapSelectHandleButton->setText(tr("? select: Handle"));
    } else {
        arapSelectFixedButton->setText(tr("select: Fixed"));
        arapSelectHandleButton->setText(tr("select: Handle"));
    }
}