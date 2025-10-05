#include "glwidget.h"
#include <QMouseEvent>
#include <cmath>
#include <limits>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      distance(5.0f),
      rotationX(0.0f),
      rotationY(0.0f),
      modelOffsetY(0.0f)
{
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    vbo.destroy();
    vao.destroy();
    doneCurrent();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!objLoader.vertices.empty()) {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = width() / (float)height();
        glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -distance);
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
        
        // 添加Y轴偏移，使模型底部对齐到屏幕底部
        glTranslatef(0.0f, modelOffsetY, 0.0f);

        // 开启线框模式
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // 设置线条颜色为白色
        glColor3f(1.0f, 1.0f, 1.0f);

        // 绘制模型
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < objLoader.indices.size(); i++) {
            const QVector3D& vertex = objLoader.vertices[objLoader.indices[i]];
            glVertex3f(vertex.x(), vertex.y(), vertex.z());
        }
        glEnd();
    }
}

void GLWidget::calculateModelBounds()
{
    if (objLoader.vertices.empty()) return;
    
    // 计算模型的精确边界框
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();
    
    for (const auto& vertex : objLoader.vertices) {
        minX = std::min(minX, vertex.x());
        maxX = std::max(maxX, vertex.x());
        minY = std::min(minY, vertex.y());
        maxY = std::max(maxY, vertex.y());
        minZ = std::min(minZ, vertex.z());
        maxZ = std::max(maxZ, vertex.z());
    }
    
    // 计算模型的整体大小
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = std::max({sizeX, sizeY, sizeZ});
    
    // 如果模型太小，进行缩放
    if (maxSize < 1.0f) {
        float scaleFactor = 2.0f / maxSize;
        for (auto& vertex : objLoader.vertices) {
            vertex = vertex * scaleFactor;
        }
        // 重新计算边界框
        minX *= scaleFactor;
        maxX *= scaleFactor;
        minY *= scaleFactor;
        maxY *= scaleFactor;
        minZ *= scaleFactor;
        maxZ *= scaleFactor;
        maxSize *= scaleFactor;
    }
    
    // 设置合适的相机距离
    distance = maxSize * 2.0f;
    if (distance < 5.0f) distance = 5.0f;
    if (distance > 100.0f) distance = 100.0f;
    
    // 计算Y轴偏移量，使模型底部对齐到视锥体底部
    // 将模型底部定位到约-0.8的位置（接近屏幕底部但不完全贴边）
    modelOffsetY = -minY - 0.8f;
    
    // 重置旋转
    rotationX = 0.0f;
    rotationY = 0.0f;
}

// 这里是加载模型的大小
bool GLWidget::loadObject(const QString& fileName)
{
    if (objLoader.loadOBJ(fileName.toStdString())) {
        calculateModelBounds();
        update();
        return true;
    }
    return false;
}

void GLWidget::resizeGL(int width, int height)
{
    float aspect = width / (float)height;
    projection.setToIdentity();
    projection.perspective(45.0f, aspect, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        rotationY += dx;
        rotationX += dy;
        update();
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    float delta = event->angleDelta().y() / 120.0f;
    distance -= delta * 0.5f;
    distance = std::max(2.0f, std::min(distance, 20.0f));
    update();
}

void GLWidget::updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices)
{
    objLoader.vertices = vertices;
    objLoader.indices = indices;
    calculateModelBounds();
    update();
}