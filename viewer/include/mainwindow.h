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
 * ����:
 *   - �ļ�����/�ָ�/����ť
 *   - ����ԭʼģ�����ڻָ�
 *- �ṩ�첽�����ź�(objLoaded)
 *   - ��ɫ/�����ʾ�л���ť
 *   - ����: ARAP�����˵�������4����ť��
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
    void openFile();       // ����ģ��
    void restoreModel();   // �ָ�ԭʼģ��
    void requestProcess(); // �����ٴδ���
    void togglePoints();   // ��ʾ/���ز�ɫ����
    void cycleColorMode(); // ��ť��ѭ���л���ɫ��ʾģʽ
    void toggleFilledFaces(); // ��ť���л������
    void toggleArapMode(); // ��ť���л�ARAPģʽ
 void clearArapFixed(); // ��ť���������ARAP�̶���
    void setArapModeFixed();  // ��ť���л���Fixedѡ��ģʽ
    void setArapModeHandle(); // ��ť���л���Handleѡ��ģʽ

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
    
    // ARAP ���߰�ť�������˵�
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