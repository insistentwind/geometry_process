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
    void clearMSTEdges(); // 新增：清除MST高亮线

    bool loadObject(const QString& fileName); // 加载OBJ文件
    const std::vector<QVector3D>& getVertices() const; // 当前顶点
    const std::vector<unsigned int>& getIndices() const; // 当前索引

    // 可选接口：切换线框/填充显示
    void setWireframe(bool enabled) { wireframe = enabled; update(); }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void calculateModelBounds();  // 计算模型边界
    
    ObjLoader objLoader;
    QOpenGLShaderProgram *program {nullptr};
    QOpenGLBuffer vertexBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuf {QOpenGLBuffer::IndexBuffer};
    QOpenGLBuffer colorBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer mstLineBuf {QOpenGLBuffer::VertexBuffer}; // 线段坐标

    // 鼠标交互
    QPoint lastPos;
    float distance;
    float rotationX;
    float rotationY;
    float modelOffsetY;  // Y轴偏移

    // 渲染数据
    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    std::vector<QVector3D> colors;
    std::vector<QVector3D> mstLineVertices; // 线段顶点对序列

    QMatrix4x4 projection; // 透视投影矩阵

    bool wireframe = true;           // 初始为线框模式
    bool perVertexColor = false;     // 初始不使用五颜六色
};

#endif // GLWIDGET_H