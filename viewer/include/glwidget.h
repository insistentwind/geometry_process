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
    
    // ��ȡ��ǰ�������ݵķ���
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
    void calculateModelBounds();  // ����ģ�ͱ߽��˽�к���
    
    ObjLoader objLoader;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QMatrix4x4 projection;
    QMatrix4x4 modelView;
    QPoint lastPos;
    float distance;
    float rotationX;
    float rotationY;
    float modelOffsetY;  // ģ��Y��ƫ���������ڽ��ײ����뵽��Ļ�ײ�
};

#endif // GLWIDGET_H