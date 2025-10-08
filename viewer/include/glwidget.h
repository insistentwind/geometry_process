#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector2D>
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
    void clearMSTEdges(); // ���MST�߶�

    bool loadObject(const QString& fileName); // ����OBJ�ļ�
    const std::vector<QVector3D>& getVertices() const; // ��ǰ����
    const std::vector<unsigned int>& getIndices() const; // ��ǰ����

    // �л��߿�
    void setWireframe(bool enabled) { wireframe = enabled; update(); }

    // ���Ʋ�ɫ�������Ⱦ������ʾ����ɫ���߿򱣳ְ�ɫ��
    void setShowColoredPoints(bool enabled) { showColoredPoints = enabled; update(); }
    void setPointSize(float psz) { pointSize = psz; update(); }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void calculateModelBounds();  // ����ģ�Ͱ�Χ��(����/�뾶)
    void resetView();              // ������ͼ����ʼ״̬

    ObjLoader objLoader;
    QOpenGLShaderProgram *program {nullptr};
    QOpenGLBuffer vertexBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuf {QOpenGLBuffer::IndexBuffer};
    QOpenGLBuffer colorBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer mstLineBuf {QOpenGLBuffer::VertexBuffer}; // MST �߶λ���

    // ����
    QPoint lastPos;
    float distance;        // �����Ŀ�����ľ���
    float rotationX;       // ������ת��
    float rotationY;       // ������ת��
    QVector2D panOffset;   // ����ͼƽ���ƽ�� (X/Y)

    // ģ�ͳ߶�
    QVector3D modelCenter; // ģ������
    float modelRadius;     // ��Χ��뾶

    // ��Ⱦ����
    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    std::vector<QVector3D> colors;
    std::vector<QVector3D> mstLineVertices; // MST�߶ζ���

    QMatrix4x4 projection; // ͸��ͶӰ����

    bool wireframe = true;           // �Ƿ��߿�
    bool perVertexColor = false;     // �Ƿ����ⲿ������ɫ
    bool showColoredPoints = true;   // �Ƿ���Ʋ�ɫ�����
    float pointSize = 6.0f;          // ������С
};

#endif // GLWIDGET_H