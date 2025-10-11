#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QVector3D>
#include <QPushButton>
#include <vector>
#include "glwidget.h"

/* --------------------------------------------------------------------------
 * MainWindow
 * 负责:
 *   - 文件打开/还原/处理按钮
 *   - 保存原始模型用于还原
 *   - 发出异步处理信号(objLoaded)
 *   - 新增: 顶点彩色点显示开关按钮
 * -------------------------------------------------------------------------- */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void updateMesh(const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices);
    void updateMeshWithColors(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices,
                              const std::vector<QVector3D>& colors);

signals:
    void objLoaded(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);

private slots:
    void openFile();       // 打开模型
    void restoreModel();   // 还原原始模型
    void requestProcess(); // 触发再次处理
    void togglePoints();   // 显示/隐藏彩色顶点点
    void cycleColorMode(); // 新增：循环颜色显示模式
    void toggleFilledFaces(); // 新增：切换填充面

private:
    void createMenus();
    void updateColorModeButtonText();
    void updateFilledFaceButtonText();

    GLWidget *glWidget { nullptr };
    QPushButton *restoreButton { nullptr };
    QPushButton *processButton { nullptr };
    QPushButton *togglePointsButton { nullptr }; // 新增按钮
    QPushButton *colorModeButton { nullptr }; // 新增：模式按钮
    QPushButton *filledFaceButton { nullptr }; // 新增按钮

    bool pointsVisible = true; // 当前彩色点显示状态

    std::vector<QVector3D> originalVertices;   // 原始顶点
    std::vector<unsigned int> originalIndices; // 原始索引
};

#endif // MAINWINDOW_H