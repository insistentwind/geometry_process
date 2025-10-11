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
    enum class ColorDisplayMode { PointsOnly, Faces }; // 颜色显示模式

    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget() override;

    void updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices);
    void updateMeshWithColors(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices,
                              const std::vector<QVector3D>& colorsIn);
    void updateMSTEdges(const std::vector<std::pair<int,int>>& edges);
    void clearMSTEdges();

    bool loadObject(const QString& fileName);
    const std::vector<QVector3D>& getVertices() const;
    const std::vector<unsigned int>& getIndices() const;

    void setWireframe(bool enabled) { wireframe = enabled; update(); }

    void setShowColoredPoints(bool enabled) { showColoredPoints = enabled; update(); }
    void setPointSize(float psz) { pointSize = psz; update(); }

    void setColorDisplayMode(ColorDisplayMode m) { colorMode = m; update(); }
    void cycleColorDisplayMode();
    ColorDisplayMode getColorDisplayMode() const { return colorMode; }

    // 新增：切换填充面显示
    void toggleFilledFaces() { showFilledFaces = !showFilledFaces; update(); }
    bool isShowingFilledFaces() const { return showFilledFaces; }

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void calculateModelBounds();
    void resetView();

    ObjLoader objLoader;
    QOpenGLShaderProgram *program {nullptr};
    QOpenGLBuffer vertexBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuf {QOpenGLBuffer::IndexBuffer};
    QOpenGLBuffer colorBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer mstLineBuf {QOpenGLBuffer::VertexBuffer};

    QPoint lastPos;
    float distance;
    float rotationX;
    float rotationY;
    QVector2D panOffset;

    QVector3D modelCenter;
    float modelRadius;

    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    std::vector<QVector3D> colors;
    std::vector<QVector3D> mstLineVertices;

    QMatrix4x4 projection;

    bool wireframe = true;
    bool perVertexColor = false;
    bool showColoredPoints = true;
    float pointSize = 6.0f;

    ColorDisplayMode colorMode { ColorDisplayMode::PointsOnly };
    bool showFilledFaces = false; // 是否显示填充面
};

#endif // GLWIDGET_H