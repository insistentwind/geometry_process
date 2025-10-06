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

signals:
    void objLoaded(const std::vector<QVector3D>& vertices,
                  const std::vector<unsigned int>& indices);

private slots:
    void openFile();
    void restoreModel();      // �ָ�ԭʼģ��
    void requestProcess();    // �����ٴδ���

private:
    GLWidget *glWidget;
    QPushButton *restoreButton{};
    QPushButton *processButton{};
    std::vector<QVector3D> originalVertices;          // ԭʼ��������
    std::vector<unsigned int> originalIndices;        // ԭʼ��������
    void createMenus();
};

#endif // MAINWINDOW_H