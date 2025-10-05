#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QFileDialog>
#include <QVector3D>
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

private:
    GLWidget *glWidget;
    void createMenus();
};

#endif // MAINWINDOW_H