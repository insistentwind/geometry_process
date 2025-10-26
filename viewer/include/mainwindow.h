#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QVector3D>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>
#include <vector>
#include "glwidget.h"

/* --------------------------------------------------------------------------
 * MainWindow
 * 功能:
 *   - 文件加载/恢复/处理按钮
 *   - 保存原始模型用于恢复
 *- 提供异步处理信号(objLoaded)
 *   - 颜色/填充显示切换按钮
 *   - 新增: ARAP下拉菜单（整合4个按钮）
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
    void openFile();       // 加载模型
    void restoreModel();   // 恢复原始模型
    void requestProcess(); // 请求再次处理
    void togglePoints();   // 显示/隐藏彩色点云
    void cycleColorMode(); // 按钮：循环切换颜色显示模式
    void toggleFilledFaces(); // 按钮：切换填充面
    void toggleArapMode(); // 按钮：切换ARAP模式
 void clearArapFixed(); // 按钮：清除所有ARAP固定点
    void setArapModeFixed();  // 按钮：切换到Fixed选择模式
    void setArapModeHandle(); // 按钮：切换到Handle选择模式

private:
    void createMenus();
    void updateColorModeButtonText();
    void updateFilledFaceButtonText();
  void updateArapButtonText();

    GLWidget *glWidget { nullptr };
    QPushButton *restoreButton { nullptr };
    QPushButton *processButton { nullptr };
    QPushButton *togglePointsButton { nullptr };
    QPushButton *colorModeButton { nullptr };
    QPushButton *filledFaceButton { nullptr };
    
    // ARAP 工具按钮和下拉菜单
    QToolButton *arapToolButton { nullptr };
    QMenu *arapMenu { nullptr };
    QAction *arapEnterExitAction { nullptr };
 QAction *arapSelectFixedAction { nullptr };
    QAction *arapSelectHandleAction { nullptr };
    QAction *arapClearFixedAction { nullptr };

    bool pointsVisible = true;

    std::vector<QVector3D> originalVertices;
    std::vector<unsigned int> originalIndices;
};

#endif // MAINWINDOW_H