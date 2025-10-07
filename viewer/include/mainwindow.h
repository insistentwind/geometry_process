#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QVector3D>
#include <QPushButton>
#include <vector>
#include "glwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateMesh(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);
    // 新增：带颜色的网格更新
    void updateMeshWithColors(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices,
                              const std::vector<QVector3D>& colors);

signals:
    void objLoaded(const std::vector<QVector3D>& vertices,
                  const std::vector<unsigned int>& indices);

private slots:
    void openFile();
    void restoreModel();      // 恢复原始模型
    void requestProcess();    // 触发再次处理

private:
    GLWidget *glWidget;
    QPushButton *restoreButton{};
    QPushButton *processButton{};
    std::vector<QVector3D> originalVertices;          // 原始顶点数据
    std::vector<unsigned int> originalIndices;        // 原始索引数据
    void createMenus();
};

#endif // MAINWINDOW_H