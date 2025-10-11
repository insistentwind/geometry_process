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
 *   - �ļ���/��ԭ/����ť
 *   - ����ԭʼģ�����ڻ�ԭ
 *   - �����첽�����ź�(objLoaded)
 *   - ����: �����ɫ����ʾ���ذ�ť
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
    void restoreModel();   // ��ԭԭʼģ��
    void requestProcess(); // �����ٴδ���
    void togglePoints();   // ��ʾ/���ز�ɫ�����
    void cycleColorMode(); // ������ѭ����ɫ��ʾģʽ
    void toggleFilledFaces(); // �������л������

private:
    void createMenus();
    void updateColorModeButtonText();
    void updateFilledFaceButtonText();

    GLWidget *glWidget { nullptr };
    QPushButton *restoreButton { nullptr };
    QPushButton *processButton { nullptr };
    QPushButton *togglePointsButton { nullptr }; // ������ť
    QPushButton *colorModeButton { nullptr }; // ������ģʽ��ť
    QPushButton *filledFaceButton { nullptr }; // ������ť

    bool pointsVisible = true; // ��ǰ��ɫ����ʾ״̬

    std::vector<QVector3D> originalVertices;   // ԭʼ����
    std::vector<unsigned int> originalIndices; // ԭʼ����
};

#endif // MAINWINDOW_H