#include "mainwindow.h"
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    // ����UI�ؼ�
    glWidget            = new GLWidget(this);
    restoreButton       = new QPushButton(tr("recover model"), this);
    processButton       = new QPushButton(tr("denoise"), this);
    togglePointsButton  = new QPushButton(tr("hide points"), this);
    colorModeButton     = new QPushButton(tr("mode: points"), this);
    filledFaceButton = new QPushButton(tr("show filled"), this);
    
    // ==================== ARAP UI�ؼ� ====================
    arapModeButton = new QPushButton(tr("enter ARAP"), this);
    arapClearFixedButton = new QPushButton(tr("clear fixed"), this);
    arapSelectFixedButton = new QPushButton(tr("select: Fixed"), this);  // ����
    arapSelectHandleButton = new QPushButton(tr("select: Handle"), this); // ����

  // ��ť����
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(restoreButton);
    buttonLayout->addWidget(processButton);
    buttonLayout->addWidget(togglePointsButton);
    buttonLayout->addWidget(colorModeButton);
    buttonLayout->addWidget(filledFaceButton);
    buttonLayout->addWidget(arapModeButton); // ARAP��ť
    buttonLayout->addWidget(arapSelectFixedButton); // ������ѡ��Fixedģʽ
    buttonLayout->addWidget(arapSelectHandleButton); // ������ѡ��Handleģʽ
    buttonLayout->addWidget(arapClearFixedButton); // ����̶��㰴ť
    buttonLayout->addStretch();

    // ������
    auto *central    = new QWidget(this);
  auto *mainLayout = new QVBoxLayout(central);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(glWidget, 1);
    central->setLayout(mainLayout);
    setCentralWidget(central);

    // �źŲ�����
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
    
    // ==================== ARAP�źŲ����� ====================
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

/* ==================== ARAP�����ۺ���ʵ�� ==================== */

/**
 * @brief �л�ARAPģʽ
 * 
 * ���ã�
 * - ����GLWidget��toggleArapMode()����/�˳�ARAPģʽ
 * - ���°�ť�ı���ʾ��ǰ״̬
 * 
 * �û��������̣�
 * 1. ���"enter ARAP"����ģʽ
 * 2. ��3D��ͼ������������ѡ��Ϊfixed���ɶ�ε����
 * 3. �ٴ����������϶�һ��������Ϊhandle���б���
 * 4. ���"exit ARAP"�˳�ģʽ
 */
void MainWindow::toggleArapMode() {
    glWidget->toggleArapMode();
    updateArapButtonText();
    updateArapSelectionButtonText(); // ͬʱ����ѡ��ģʽ��ť
}

/**
 * @brief ���������ѡ��Ĺ̶�����
 * 
 * ���ã�
 * - ����GLWidget��clearArapFixedVertices()
 * - ���MeshProcessor�����ж����fixed���
 * - ���û�����ѡ��Լ����
 * 
 * ʹ�ó�����
 * - �û��Ե�ǰѡ���fixed�㲻���⣬������ѡ��
 */
void MainWindow::clearArapFixed() {
    glWidget->clearArapFixedVertices();
}

/**
 * @brief ѡ��Fixed����
 * 
 * ���ã�
 * - �л����̶���ѡ��ģʽ
 * - �û���3D��ͼ�е������ʱ��ѡ�еĶ��㽫�����Ϊfixed
 * - ֧�ֶ�ε�����û���ѡ����fixed��
 * 
 * �û��������̣�
 * 1. ���"select: Fixed"����ѡ��̶���ģʽ
 * 2. ��3D��ͼ��������һ�������������ѡ��
 * 3. ���������ť���ٴε��"select: Fixed"�˳�ģʽ
 */
void MainWindow::setArapModeFixed() {
    // �л����̶���ѡ��ģʽ
    glWidget->setArapSelectionMode(GLWidget::ArapSelectionMode::SelectFixed);
    updateArapSelectionButtonText();
}

/**
 * @brief ѡ��Handle����
 * 
 * ���ã�
 * - �л���Handle��ѡ��ģʽ
 * - �û���3D��ͼ�е������ʱ��ѡ�еĶ��㽫�����Ϊhandle
 * - ֻ��һ������ᱻ���Ϊhandle����ε�������Ϊ����ѡ��
 * 
 * �û��������̣�
 * 1. ���"select: Handle"����ѡ��Handle��ģʽ
 * 2. ��3D��ͼ��������һ���������ѡ��
 * 3. ���������ť���ٴε��"select: Handle"�˳�ģʽ
 */
void MainWindow::setArapModeHandle() {
    // �л���Handle��ѡ��ģʽ
    glWidget->setArapSelectionMode(GLWidget::ArapSelectionMode::SelectHandle);
    updateArapSelectionButtonText();
}

/**
 * @brief ����ARAP��ť�ı�
 * 
 * ���ݵ�ǰ�Ƿ���ARAPģʽ����ʾ��
 * - "enter ARAP"����ARAPģʽ���������
 * - "exit ARAP"��ARAPģʽ�У�����˳�
 */
void MainWindow::updateArapButtonText() {
    arapModeButton->setText(glWidget->isArapActive() ? tr("exit ARAP") : tr("enter ARAP"));
    
    // ����ARAPģʽ״̬����/����ѡ��ť
    bool arapActive = glWidget->isArapActive();
    arapSelectFixedButton->setEnabled(arapActive);
    arapSelectHandleButton->setEnabled(arapActive);
    arapClearFixedButton->setEnabled(arapActive);
}

/**
 * @brief ����ѡ��ģʽ��ť�ı�
 * ���ݵ�ǰѡ��ģʽ������Ӧ��ť
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