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
 *   - 文件打开/恢复/触发处理按钮
 *   - 保存原始模型数据用于恢复
 *   - 触发异步处理信号(objLoaded)
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
    void restoreModel();   // 恢复原始模型
    void requestProcess(); // 触发再次处理

private:
    void createMenus();

    GLWidget *glWidget { nullptr };
    QPushButton *restoreButton { nullptr };
    QPushButton *processButton { nullptr };

    std::vector<QVector3D> originalVertices;   // 原始顶点
    std::vector<unsigned int> originalIndices; // 原始索引
};

#endif // MAINWINDOW_H