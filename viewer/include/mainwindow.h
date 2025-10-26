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
 *   - 顶点彩色点显示开关按钮
 *   - 新增: ARAP模式切换和固定点清除
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
    void toggleArapMode(); // 新增：切换ARAP模式
    void clearArapFixed(); // 新增：清除ARAP固定点
    void setArapModeFixed();  // 新增：切换到Fixed选择模式
    void setArapModeHandle(); // 新增：切换到Handle选择模式

private:
    void createMenus();
    void updateColorModeButtonText();
    void updateFilledFaceButtonText();
    void updateArapButtonText(); // 新增：更新ARAP按钮文本
    void updateArapSelectionButtonText(); // 新增：更新选择模式按钮文本

    GLWidget *glWidget { nullptr };
    QPushButton *restoreButton { nullptr };
    QPushButton *processButton { nullptr };
    QPushButton *togglePointsButton { nullptr }; // 新增按钮
    QPushButton *colorModeButton { nullptr }; // 新增：模式按钮
    QPushButton *filledFaceButton { nullptr }; // 新增按钮
    QPushButton *arapModeButton { nullptr };  // ARAP模式切换按钮
    QPushButton *arapClearFixedButton { nullptr }; // 清除固定点按钮
    QPushButton *arapSelectFixedButton { nullptr }; // 新增：选择Fixed模式按钮
    QPushButton *arapSelectHandleButton { nullptr }; // 新增：选择Handle模式按钮

    bool pointsVisible = true; // 当前彩色点显示状态

    std::vector<QVector3D> originalVertices;   // 原始顶点
    std::vector<unsigned int> originalIndices; // 原始索引
};

#endif // MAINWINDOW_H