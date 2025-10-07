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
    void updateMeshWithColors(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices,
                              const std::vector<QVector3D>& colorsIn);
    void updateMSTEdges(const std::vector<std::pair<int,int>>& edges);
    void clearMSTEdges(); // ���������MST������

    bool loadObject(const QString& fileName); // ����OBJ�ļ�
    const std::vector<QVector3D>& getVertices() const; // ��ǰ����
    const std::vector<unsigned int>& getIndices() const; // ��ǰ����

    // ��ѡ�ӿڣ��л��߿�/�����ʾ
    void setWireframe(bool enabled) { wireframe = enabled; update(); }

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
    QOpenGLBuffer mstLineBuf {QOpenGLBuffer::VertexBuffer}; // �߶�����

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
    std::vector<QVector3D> mstLineVertices; // �߶ζ��������

    QMatrix4x4 projection; // ͸��ͶӰ����

    bool wireframe = true;           // ��ʼΪ�߿�ģʽ
    bool perVertexColor = false;     // ��ʼ��ʹ��������ɫ
};

#endif // GLWIDGET_H