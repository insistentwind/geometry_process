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
    void clearMSTEdges(); // 清除MST线段

    bool loadObject(const QString& fileName); // 加载OBJ文件
    const std::vector<QVector3D>& getVertices() const; // 当前顶点
    const std::vector<unsigned int>& getIndices() const; // 当前索引

    // 切换线框
    void setWireframe(bool enabled) { wireframe = enabled; update(); }

    // 控制彩色顶点点渲染（仅显示点颜色，线框保持白色）
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
    void calculateModelBounds();  // 计算模型包围盒(中心/半径)
    void resetView();              // 重置视图到初始状态

    ObjLoader objLoader;
    QOpenGLShaderProgram *program {nullptr};
    QOpenGLBuffer vertexBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuf {QOpenGLBuffer::IndexBuffer};
    QOpenGLBuffer colorBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer mstLineBuf {QOpenGLBuffer::VertexBuffer}; // MST 线段缓冲

    // 交互
    QPoint lastPos;
    float distance;        // 相机到目标中心距离
    float rotationX;       // 上下旋转角
    float rotationY;       // 左右旋转角
    QVector2D panOffset;   // 在视图平面的平移 (X/Y)

    // 模型尺度
    QVector3D modelCenter; // 模型中心
    float modelRadius;     // 包围球半径

    // 渲染数据
    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    std::vector<QVector3D> colors;
    std::vector<QVector3D> mstLineVertices; // MST线段顶点

    QMatrix4x4 projection; // 透视投影矩阵

    bool wireframe = true;           // 是否线框
    bool perVertexColor = false;     // 是否有外部顶点颜色
    bool showColoredPoints = true;   // 是否绘制彩色顶点点
    float pointSize = 6.0f;          // 顶点点大小
};

#endif // GLWIDGET_H