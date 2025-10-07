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
 * ����:
 *   - �ļ���/�ָ�/��������ť
 *   - ����ԭʼģ���������ڻָ�
 *   - �����첽�����ź�(objLoaded)
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
    void openFile();       // ��ģ��
    void restoreModel();   // �ָ�ԭʼģ��
    void requestProcess(); // �����ٴδ���

private:
    void createMenus();

    GLWidget *glWidget { nullptr };
    QPushButton *restoreButton { nullptr };
    QPushButton *processButton { nullptr };

    std::vector<QVector3D> originalVertices;   // ԭʼ����
    std::vector<unsigned int> originalIndices; // ԭʼ����
};

#endif // MAINWINDOW_H