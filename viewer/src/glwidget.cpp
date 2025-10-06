#include "glwidget.h"
#include <QMouseEvent>
#include <cmath>
#include <limits>

/*
 * GLWidget.cpp ���
 * ------------------
 * ���ļ�ʵ���˻��ھ�ʽ�̶�����(OpenGL 1.x ���) ��һ���� 3D ģ�Ͳ鿴�����
 * ��Ҫְ��
 *   1. ����ģ�ͼ�������(ͨ���ڲ��� ObjLoader ���涥��������)��
 *   2. ���ݴ��ڳߴ�����ͶӰ���� (ʹ�� glFrustum ģ��͸��ͶӰ)��
 *   3. �� paintGL ��ִ��ģ����ͼ�����ջ���������� -> ƽ��(��Զ�������) -> ��ת -> Y ��У��ƫ�ơ�
 *   4. ��ģ�ͽ��м򵥵�����Ӧ���������/�ײ����봦�� (calculateModelBounds)��
 *   5. �ṩ����������
 *        - �������϶����� X/Y ����תģ��
 *        - �����֣��ı� distance��ʵ������ (�������� Z ��ƽ��)
 *   6. ֧���ⲿ���� updateMesh �滻��ǰ��ʾ�ļ�������(���羭���㷨������ƽ��ģ��)��
 *
 * ��Ҫ��Ա���壺
 *   - distance:      ������ Z ������ƽ�Ƶľ��룬ģ�⡰������ơ���
 *   - rotationX/Y:   ����������ģ����ת�Ƕ� (�� X / Y ��)��
 *   - modelOffsetY:  Y �������ƽ�ƣ�ʹģ�͵�������ͼ�·����룬�����Ӿ����顣
 *   - objLoader:     ��ż��ص� .obj ����(vertex) ������(indices)��
 *
 * ������Ⱦ���� (paintGL):
 *   1. ���� (��ɫ + ���)
 *   2. ����ͶӰ���� (glFrustum)
 *   3. ����ģ����ͼ���� (glLoadIdentity)
 *   4. glTranslatef(0,0,-distance) �ѡ��������Զ
 *   5. glRotatef ���ӽ�����ת
 *   6. glTranslatef(0, modelOffsetY, 0) ��ģ�͵ײ����Ƶ���������ײ�
 *   7. �߿�ģʽ + ��ɫ
 *   8. glBegin(GL_TRIANGLES) + ������������
 *
 * ע�⣺��ʵ��ʹ���˹̶����ܹ��� (Immediate Mode)���ִ� OpenGL �Ƽ�ʹ�� VAO/VBO + ��ɫ����
 * �ڽ�ѧ / ����������֤�������Կ�ʹ�����ַ�ʽ�����������ڸ����ܻ����չ��Ⱦϵͳ��
 */

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      distance(1.0f),        // ��ʼ���������Զ����
      rotationX(0.0f),       // �� X ���ʼ��ת��
      rotationY(0.0f),       // �� Y ���ʼ��ת��
      modelOffsetY(0.0f)     // ģ�� Y ����ƫ�Ƴ�ʼ��
{
    // ����ò�����ȡ���̽��㼰����¼�
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    // ȷ���ڵ�ǰ OpenGL �����������ٻ������ (��Ȼ��ǰ����δ����ʹ�� VBO/VAO ����)
    makeCurrent();
    vbo.destroy();
    vao.destroy();
    doneCurrent();
}

void GLWidget::initializeGL()
{
    // ��ʼ�� OpenGL ����ָ�� (�� QOpenGLFunctions �ṩ)
    initializeOpenGLFunctions();

    // ����״̬������ɫ + ��Ȳ���
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::paintGL()
{
    // 1. �����ɫ����Ȼ��壬׼����һ֡
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // û�ж������ݾͲ�����
    if (!objLoader.vertices.empty()) {
        // 2. ---- ����ͶӰ���� ----
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity(); // ����ͶӰ����
        float aspect = width() / (float)height();
        // ʹ�öԳ���׶�壬near=1, far=100��aspect �������ҷ�Χ
        glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 100.0);

        // 3. ---- ����ģ����ͼ���� ----
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity(); // ����ģ����ͼ����
        // ������������Z�Ḻ����ƽ�� distance���൱�ڰѡ����������������ȷ��ģ�ʹ��ڿ��ӷ�Χ��
        glTranslatef(0.0f, 0.0f, -distance);
        // ��ת˳������ X������ Y�������켣������ת����
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
        
        // ���� Y ��ƫ�ƣ���ģ�͵ײ��ƶ�����Ļ�ϵ�λ�ã���������Եá�Ư����
        glTranslatef(0.0f, modelOffsetY, 0.0f);

        // 4. �߿���Ⱦ���ڲ鿴���˽ṹ
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1.0f, 1.0f, 1.0f); // ��ɫ�߿�

        // 5. Immediate Mode ����������
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
    
    // �������ж��㣬���� AABB (������Χ��)
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
    
    // ��������ߴ磬���ڹ��ơ���������룬�Լ���Ҫʱ������
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = std::max({sizeX, sizeY, sizeZ});
    
    // ��ģ�ͳ߶ȹ�С��ͳһ�Ŵ󵽽��ƴ�С 2.0 �ķ�Χ��������ʾ��С
    if (maxSize < 1.0f) {
        float scaleFactor = 2.0f / maxSize; // Ŀ�귶Χ ~2.0
        for (auto& vertex : objLoader.vertices) {
            vertex = vertex * scaleFactor;
        }
        // ����ֱ��������ԭ���ݣ�����ͬ�����ű߽�ֵ (�����ٴα���)
        minX *= scaleFactor; maxX *= scaleFactor;
        minY *= scaleFactor; maxY *= scaleFactor;
        minZ *= scaleFactor; maxZ *= scaleFactor;
        maxSize *= scaleFactor;
    }
    
    // �������߶ȹ�����룬ʹģ�ʹ���ռ����Ұ
    distance = maxSize * 2.0f; // ����ϵ��
    if (distance < 0.0f)  distance = 1.0f;   // ���޷�ֹ�����ü���
    if (distance > 100.0f) distance = 100.0f; // ���ޱ����Զ
    
    // ���� Y ��ƫ�ƣ�ʹ�ײ�λ�ýӽ���׶���·� (-0.8 ~ ����ֵ)
    modelOffsetY = -minY - 0.8f;
    
    // ����/���º����ý�����ת
    rotationX = 0.0f;
    rotationY = 0.0f;
}

// ���� OBJ �ļ����ɹ�������Χ�в������ػ�
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
    // ���������һ�� QMatrix4x4 ��ʽ��ͶӰ���� (��ǰ��Ⱦ�̶�����δֱ��ʹ��)
    float aspect = width / (float)height;
    projection.setToIdentity();
    projection.perspective(45.0f, aspect, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    // ��¼��ʼ���λ�ã�����֮����϶���������
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x(); // ˮƽλ��
    int dy = event->y() - lastPos.y(); // ��ֱλ��

    if (event->buttons() & Qt::LeftButton) {
        // ��ӳ�䣺ˮƽ�϶� -> Y ����ת����ֱ�϶� -> X ����ת
        rotationY += dx;
        rotationX += dy;
        update(); // �����ػ�
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    // angleDelta().y() ÿ���̶� 120������Ϊ���Ų���
    float delta = event->angleDelta().y() / 120.0f;
    distance -= delta * 0.5f; // ��ǰ���� -> ����
    // ���Ʒ�Χ�����⴩���ü�����Զ
    distance = std::max(2.0f, std::min(distance, 20.0f));
    update();
}

void GLWidget::updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices)
{
    // �滻��ǰģ������ (ͨ�������㷨������)
    objLoader.vertices = vertices;
    objLoader.indices  = indices;
    calculateModelBounds(); // ���¼���ߴ���ƫ��
    update();               // �����ػ�
}