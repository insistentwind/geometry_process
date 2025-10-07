#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include "objloader.h"
#include <QOpenGLShaderProgram>
#include <QString>
#include <vector>

class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget() override;

    void updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices);
    bool loadObject(const QString& fileName); // ����OBJ�ļ�
    const std::vector<QVector3D>& getVertices() const; // ��ǰ����
    const std::vector<unsigned int>& getIndices() const; // ��ǰ����

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void calculateModelBounds();  // ����ģ�ͱ߽�
    
    ObjLoader objLoader;
    QOpenGLShaderProgram *program {nullptr};
    QOpenGLBuffer vertexBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuf {QOpenGLBuffer::IndexBuffer};
    QOpenGLBuffer colorBuf {QOpenGLBuffer::VertexBuffer};

    // ��꽻��
    QPoint lastPos;
    float distance;
    float rotationX;
    float rotationY;
    float modelOffsetY;  // Y��ƫ��

    // ��Ⱦ����
    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    std::vector<QVector3D> colors;

    QMatrix4x4 projection; // ͸��ͶӰ����
};

#endif // GLWIDGET_H