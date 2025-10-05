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
        
        // ���Y��ƫ�ƣ�ʹģ�͵ײ����뵽��Ļ�ײ�
        glTranslatef(0.0f, modelOffsetY, 0.0f);

        // �����߿�ģʽ
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // ����������ɫΪ��ɫ
        glColor3f(1.0f, 1.0f, 1.0f);

        // ����ģ��
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
    
    // ����ģ�͵ľ�ȷ�߽��
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
    
    // ����ģ�͵������С
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = std::max({sizeX, sizeY, sizeZ});
    
    // ���ģ��̫С����������
    if (maxSize < 1.0f) {
        float scaleFactor = 2.0f / maxSize;
        for (auto& vertex : objLoader.vertices) {
            vertex = vertex * scaleFactor;
        }
        // ���¼���߽��
        minX *= scaleFactor;
        maxX *= scaleFactor;
        minY *= scaleFactor;
        maxY *= scaleFactor;
        minZ *= scaleFactor;
        maxZ *= scaleFactor;
        maxSize *= scaleFactor;
    }
    
    // ���ú��ʵ��������
    distance = maxSize * 2.0f;
    if (distance < 5.0f) distance = 5.0f;
    if (distance > 100.0f) distance = 100.0f;
    
    // ����Y��ƫ������ʹģ�͵ײ����뵽��׶��ײ�
    // ��ģ�͵ײ���λ��Լ-0.8��λ�ã��ӽ���Ļ�ײ�������ȫ���ߣ�
    modelOffsetY = -minY - 0.8f;
    
    // ������ת
    rotationX = 0.0f;
    rotationY = 0.0f;
}

// �����Ǽ���ģ�͵Ĵ�С
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