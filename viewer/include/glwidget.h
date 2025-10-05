#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include "objloader.h"

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    bool loadObject(const QString &fileName);
    void updateMesh(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);
    
    // 获取当前网格数据的方法
    const std::vector<QVector3D>& getVertices() const { return objLoader.vertices; }
    const std::vector<unsigned int>& getIndices() const { return objLoader.indices; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void calculateModelBounds();  // 计算模型边界的私有函数
    
    ObjLoader objLoader;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QMatrix4x4 projection;
    QMatrix4x4 modelView;
    QPoint lastPos;
    float distance;
    float rotationX;
    float rotationY;
    float modelOffsetY;  // 模型Y轴偏移量，用于将底部对齐到屏幕底部
};

#endif // GLWIDGET_H