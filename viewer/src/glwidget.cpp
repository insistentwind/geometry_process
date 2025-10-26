#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QDebug>
#include <QtCore/QResource>
#include <QFile>
#include <cmath>
#include <limits>
#include <iostream> // ���iostream����std::cout

/* --------------------------------------------------------------------------
 * GLWidget �Ľ���: ��ӹ�����(���Ļ�, ƽ��, ��ƽ������) + ARAP����
 * -------------------------------------------------------------------------- */

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent),
  distance(4.0f),
      rotationX(0.0f),
      rotationY(0.0f),
      panOffset(0.0f, 0.0f),
      modelCenter(0,0,0),
      modelRadius(1.0f) {
    Q_INIT_RESOURCE(resources);
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget() {
    makeCurrent();
    vertexBuf.destroy();
    indexBuf.destroy();
    colorBuf.destroy();
    mstLineBuf.destroy();
    delete program;
    doneCurrent();
}

void GLWidget::cycleColorDisplayMode() {
    if (colorMode == ColorDisplayMode::PointsOnly)
        colorMode = ColorDisplayMode::Faces;
    else
        colorMode = ColorDisplayMode::PointsOnly;
    update();
}

/* ---------------------------- ���ݸ���: ������ ---------------------------- */
void GLWidget::updateMesh(const std::vector<QVector3D>& v,
                          const std::vector<unsigned int>& idx) {
    vertices = v;
    indices  = idx;
    perVertexColor = false;
    colors.assign(vertices.size(), QVector3D(1.0f, 1.0f, 1.0f));

    if (!isValid()) return;

    vertexBuf.bind();
    vertexBuf.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(QVector3D)));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));

    colorBuf.bind();
    colorBuf.allocate(colors.data(), static_cast<int>(colors.size() * sizeof(QVector3D)));

    calculateModelBounds();
    resetView();
    update();
}

/* ---------------------------- ���ݸ���: ����ɫ ----------------------------- */
void GLWidget::updateMeshWithColors(const std::vector<QVector3D>& v,
                                    const std::vector<unsigned int>& idx,
                                    const std::vector<QVector3D>& cols) {
    vertices = v;
    indices  = idx;

    if (cols.size() == v.size()) {
        colors = cols;
        perVertexColor = true;
    } else {
        colors.assign(v.size(), QVector3D(1.0f, 1.0f, 1.0f));
        perVertexColor = false;
    }

    if (!isValid()) return;

    vertexBuf.bind();
    vertexBuf.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(QVector3D)));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));

    colorBuf.bind();
    colorBuf.allocate(colors.data(), static_cast<int>(colors.size() * sizeof(QVector3D)));

    calculateModelBounds();
    resetView();
    update();
}

/* ------------------------------ ���� MST �߶� ----------------------------- */
void GLWidget::updateMSTEdges(const std::vector<std::pair<int, int>>& edges) {
    mstLineVertices.clear();
    mstLineVertices.reserve(edges.size() * 2);

    for (const auto& e : edges) {
        int a = e.first;
        int b = e.second;
        if (a >= 0 && b >= 0 && a < static_cast<int>(vertices.size()) && b < static_cast<int>(vertices.size())) {
            mstLineVertices.push_back(vertices[a]);
            mstLineVertices.push_back(vertices[b]);
        }
    }

    if (!mstLineBuf.isCreated()) {
        mstLineBuf.create();
    }
    mstLineBuf.bind();
    mstLineBuf.allocate(mstLineVertices.data(), static_cast<int>(mstLineVertices.size() * sizeof(QVector3D)));
    update();
}

/* ------------------------------ ��� MST ���� ------------------------------ */
void GLWidget::clearMSTEdges() {
    mstLineVertices.clear();
    if (mstLineBuf.isCreated()) {
        mstLineBuf.bind();
        mstLineBuf.allocate(nullptr, 0);
    }
    update();
}

/* ------------------------------- ���ʽӿ� ------------------------------- */
const std::vector<QVector3D>& GLWidget::getVertices() const { return vertices; }
const std::vector<unsigned int>& GLWidget::getIndices() const { return indices; }

/* ------------------------------- Fallback Shader -------------------------- */
static const char* fallbackVert = R"GLSL(
#version 330 core
layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_color;
uniform mat4 u_mvp;
out vec3 vColor;
void main(){
    vColor = a_color;
    gl_Position = u_mvp * vec4(a_position, 1.0);
}
)GLSL";

static const char* fallbackFrag = R"GLSL(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main(){ FragColor = vec4(vColor, 1.0); }
)GLSL";

/* ----------------------------- OpenGL ��ʼ�� ------------------------------ */
void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

    vertexBuf.create();
    indexBuf.create();
    colorBuf.create();
    mstLineBuf.create();

    vertexBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    indexBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    colorBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    mstLineBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    program = new QOpenGLShaderProgram(this);
    bool okV = false;
    bool okF = false;

    if (QFile(":/shaders/basic.vert").exists()) {
        okV = program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/basic.vert");
    }
    if (QFile(":/shaders/basic.frag").exists()) {
        okF = program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/basic.frag");
    }
    if (!okV) program->addShaderFromSourceCode(QOpenGLShader::Vertex, fallbackVert);
    if (!okF) program->addShaderFromSourceCode(QOpenGLShader::Fragment, fallbackFrag);

    program->bindAttributeLocation("a_position", 0);
    program->bindAttributeLocation("a_color", 1);

    if (!program->link()) {
        qWarning() << "Shader link failed" << program->log();
    }
}

/* --------------------------------- ���� ---------------------------------- */
void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (vertices.empty() || !program || !program->isLinked()) return;

    // �߿�ģʽʼ�ձ��֣�����������������ѡ����
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    float aspect = (width() > 0 && height() > 0) ? (float)width() / (float)height() : 1.0f;
    float nearPlane = std::max(0.001f, modelRadius * 0.01f);
    float farPlane  = std::max(1000.0f * nearPlane, distance + modelRadius * 4.0f);
    QMatrix4x4 proj;  proj.perspective(45.0f, aspect, nearPlane, farPlane);

    float rx = qDegreesToRadians(rotationX);
    float ry = qDegreesToRadians(rotationY);
    float cosX = std::cos(rx);

    QVector3D camPos(
        distance * std::sin(ry) * cosX,
        distance * std::sin(rx),
        distance * std::cos(ry) * cosX
    );

    QVector3D target = modelCenter + QVector3D(panOffset.x(), panOffset.y(), 0.0f);
    QVector3D up = (cosX >= 0.0f) ? QVector3D(0,1,0) : QVector3D(0,-1,0);

    QMatrix4x4 view; view.lookAt(camPos + target, target, up);
    QMatrix4x4 mvp = proj * view;

    program->bind();
    program->setUniformValue("u_mvp", mvp);

    // �ȿ�ѡ��������� (GL_FILL)���ٻ����߿� (GL_LINE) ������߿ɼ���
    if (showFilledFaces) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        vertexBuf.bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        if (perVertexColor && colorMode == ColorDisplayMode::Faces) {
            colorBuf.bind();
            program->enableAttributeArray(1);
            program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(QVector3D));
        } else {
            program->disableAttributeArray(1);
            glVertexAttrib3f(1, 0.85f, 0.85f, 0.85f); // Ĭ�ϵ���
        }
        indexBuf.bind();
        glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    // �ٻ����߿�
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    vertexBuf.bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    program->disableAttributeArray(1);
    glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);
    indexBuf.bind();
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);

    // MST ��
    if (!mstLineVertices.empty()) {
        glLineWidth(3.0f);
        program->disableAttributeArray(1);
        glVertexAttrib3f(1, 1.0f, 0.0f, 0.0f);
        mstLineBuf.bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        glDrawArrays(GL_LINES, 0, static_cast<int>(mstLineVertices.size()));
        glLineWidth(1.0f);
    }

    // ����
    if (showColoredPoints) {
        glPointSize(pointSize);
        vertexBuf.bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        if (perVertexColor) {
            colorBuf.bind();
            program->enableAttributeArray(1);
            program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(QVector3D));
        } else {
            program->disableAttributeArray(1);
            glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);
        }
        glDrawArrays(GL_POINTS, 0, static_cast<int>(vertices.size()));
    }

    // ARAPģʽ��������ʾ����fixed���㣨��ɫ��㣩
    if (arapActive && !fixedVertices.empty()) {
    glPointSize(12.0f);  // �ϴ�ĵ�
        vertexBuf.bind();
      program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        program->disableAttributeArray(1);
glVertexAttrib3f(1, 0.2f, 0.5f, 1.0f);  // ��ɫ

        // ��������fixed����
        for (int vertexIndex : fixedVertices) {
            if (vertexIndex >= 0 && vertexIndex < static_cast<int>(vertices.size())) {
   glDrawArrays(GL_POINTS, vertexIndex, 1);
     }
        }
    }
    
    // ARAPģʽ������handle���㣨��ɫ�㣩
    if (arapActive && arapHandleVertex >= 0) {
   glPointSize(15.0f);  // ����ĵ�
  vertexBuf.bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        program->disableAttributeArray(1);
        glVertexAttrib3f(1, 1.0f, 0.2f, 0.2f);  // ��ɫ
        
 glDrawArrays(GL_POINTS, arapHandleVertex, 1);
    }

    program->disableAttributeArray(0);
    program->disableAttributeArray(1);
    program->release();
}

/* ------------------------------- �����Χ�� ------------------------------- */
void GLWidget::calculateModelBounds() {
    if (vertices.empty()) return;

    float minX =  std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float minY =  std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float minZ =  std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (const auto& v : vertices) {
        minX = std::min(minX, v.x()); maxX = std::max(maxX, v.x());
        minY = std::min(minY, v.y()); maxY = std::max(maxY, v.y());
        minZ = std::min(minZ, v.z()); maxZ = std::max(maxZ, v.z());
    }

    modelCenter = QVector3D((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, (minZ + maxZ) * 0.5f);
    QVector3D extents(maxX - minX, maxY - minY, maxZ - minZ);
    modelRadius = std::max({ extents.x(), extents.y(), extents.z() }) * 0.5f;
    if (modelRadius < 1e-6f) modelRadius = 1.0f;
}

void GLWidget::resetView() {
    rotationX = 0.0f;
    rotationY = 0.0f;
    panOffset = QVector2D(0,0);
    distance = modelRadius * 3.5f;  // ���ӵ�3.5����ʹģ����ʾ��С
    distance = std::clamp(distance, 0.5f * modelRadius, 10.0f * modelRadius);
}

/* ------------------------------- ���� OBJ �ļ� ---------------------------- */
bool GLWidget::loadObject(const QString& file) {
    if (objLoader.loadOBJ(file.toStdString())) {
        updateMesh(objLoader.vertices, objLoader.indices);
        mstLineVertices.clear();
        return true;
    }
    return false;
}

/* ------------------------------- ���ڳߴ�仯 ----------------------------- */
void GLWidget::resizeGL(int w, int h) {
    projection.setToIdentity();
    if (h > 0) {
        projection.perspective(45.0f, static_cast<float>(w) / static_cast<float>(h), 0.01f, 100.0f);
    }
}

/* --------------------------------- �����¼� -------------------------------- */
void GLWidget::mousePressEvent(QMouseEvent* e) {
 lastPos = e->pos();

 // ARAPģʽ�µ�������������ѡ��ģʽ������Ϊ
 if (arapActive && e->button() == Qt::LeftButton) {
 int hitVertex = pickVertex(e->pos());
 if (hitVertex >=0) {
 if (arapSelectionMode == ArapSelectionMode::SelectFixed) {
 // === Fixed��ѡ��ģʽ����ӵ�fixed���� ===
 fixedVertices.insert(hitVertex);
 // ���ûص����Ϊfixed
 if (arapSetFixedCallback) {
 arapSetFixedCallback(hitVertex, true);
 std::cout << "[GLWidget] Marked vertex " << hitVertex << " as FIXED (total: "
 << fixedVertices.size() << " fixed vertices)" << std::endl;
 }
 }
 else if (arapSelectionMode == ArapSelectionMode::SelectHandle) {
 // === Handle��ѡ��ģʽ������Ϊ�϶��� ===
 arapHandleVertex = hitVertex;
 //�������λ��
 float rx = qDegreesToRadians(rotationX);
 float ry = qDegreesToRadians(rotationY);
 float cosX = std::cos(rx);
 QVector3D camPos(
 distance * std::sin(ry) * cosX,
 distance * std::sin(rx),
 distance * std::cos(ry) * cosX
 );
 QVector3D target = modelCenter + QVector3D(panOffset.x(), panOffset.y(),0.0f);
 camPos += target;
 //��¼handle��ȣ����������
 handleDepth = (vertices[hitVertex] - camPos).length();
 //������� mouseMoveEvent������ק
 leftButtonDraggingHandle = true;
 std::cout << "[GLWidget] Selected vertex " << hitVertex << " as HANDLE (ready to drag)" << std::endl;
 }
 }
 return; // ARAPģʽ�£��������ת��ͼ
 }
 //�Ҽ�������ͼ
 if (e->button() == Qt::RightButton) {
 resetView();
 update();
 }
}

void GLWidget::mouseMoveEvent(QMouseEvent* e) {
 int dx = e->x() - lastPos.x();
 int dy = e->y() - lastPos.y();

 if (arapActive) {
 // ����ѡ�� handle ������԰�����ִ����ק
 if ((e->buttons() & Qt::LeftButton) && arapHandleVertex >=0 && leftButtonDraggingHandle) {
 performArapDrag(e->pos());
 }
 } else {
 if (e->buttons() & Qt::LeftButton) {
 rotationY -= dx *0.5f;
 rotationX += dy *0.5f;
 if (rotationX >179.f) rotationX -=360.f;
 if (rotationX < -179.f) rotationX +=360.f;
 update();
 } else if (e->buttons() & Qt::MiddleButton) {
 float panScale = distance *0.0015f;
 panOffset += QVector2D(-dx * panScale, dy * panScale);
 update();
 }
 }
 lastPos = e->pos();
}

void GLWidget::mouseReleaseEvent(QMouseEvent* e) {
 if (e->button() == Qt::LeftButton) {
 if (arapActive && leftButtonDraggingHandle) {
 leftButtonDraggingHandle = false;
 std::cout << "[GLWidget] Stopped dragging handle" << std::endl;
 }
 }
}

void GLWidget::wheelEvent(QWheelEvent* e) {
    float steps = e->angleDelta().y() / 120.0f;
    float factor = std::pow(0.9f, steps);
    distance *= factor;
  float minDist = 0.1f * modelRadius;
    float maxDist = 20.0f * modelRadius;
    distance = std::clamp(distance, minDist, maxDist);
    update();
}

/* ==================== ARAP��������ʵ�� ==================== */

/**
 * @brief �л�ARAPģʽ
 * 
 * ����ARAPģʽʱ��
 * - ����arapBeginCallback��ʼ��������old_position��
 * - ������ѡ��fixed����
 * - �����קִ��ARAP����
 * 
 * �˳�ARAPģʽʱ��
 * - ����handle��������
 * - �ָ������������
 */
void GLWidget::toggleArapMode() {
  arapActive = !arapActive;
    
    if (!arapActive) {
        // �˳�ARAPģʽ������״̬
    fixedVertices.clear();
      arapHandleVertex = -1;
      leftButtonDraggingHandle = false;
        arapSelectionMode = ArapSelectionMode::None; // ����ѡ��ģʽ
  std::cout << "[GLWidget] Exited ARAP mode" << std::endl;
    } else {
      // ����ARAPģʽ����ʼ����ֻ����һ�Σ�
     if (arapBeginCallback) {
    arapBeginCallback();
        }
        arapSelectionMode = ArapSelectionMode::SelectFixed; // Ĭ�Ͻ���Fixedѡ��ģʽ
    std::cout << "[GLWidget] Entered ARAP mode - Use toolbar buttons to select Fixed/Handle vertices" << std::endl;
    }

    update();
}

/**
 * @brief ���������ѡ��Ĺ̶�����
 */
void GLWidget::clearArapFixedVertices() {
    if (arapClearFixedCallback) {
        arapClearFixedCallback();
    }
    
    // ���fixed���㼯��
    fixedVertices.clear();
    
// ����handle״̬
    arapHandleVertex = -1;
    leftButtonDraggingHandle = false;
    
    std::cout << "[GLWidget] Cleared all fixed vertices" << std::endl;
    update();
}

/**
 * @brief ��ʼARAP�Ự������ص������ã�
 */
void GLWidget::beginArapIfNeeded() {
    if (arapBeginCallback) {
    arapBeginCallback();
        std::cout << "[GLWidget] ARAP session initialized" << std::endl;
    }
}

/**
 * @brief ��Ļ����ʰȡ����Ķ���
 * @param pos �����Ļ����
 * @return ����������-1��ʾδʰȡ��
 * 
 * ʵ��ԭ��
 * 1. ����MVP����ģ��-��ͼ-ͶӰ����
 * 2. �����ж���任����Ļ�ռ�
 * 3. �������λ����ÿ���������Ļ����
 * 4. ���ؾ����������15������ֵ�ڵĶ���
 */
int GLWidget::pickVertex(const QPoint& pos) const {
    if (vertices.empty()) return -1;
    
    // ����MVP������paintGL����ͬ��
    float aspect = (width() > 0 && height() > 0) ? (float)width() / (float)height() : 1.0f;
QMatrix4x4 proj;
    proj.perspective(45.0f, aspect, 0.01f, 1000.0f);
    
    float rx = qDegreesToRadians(rotationX);
    float ry = qDegreesToRadians(rotationY);
    float cosX = std::cos(rx);
    
    QVector3D camPos(
        distance * std::sin(ry) * cosX,
        distance * std::sin(rx),
        distance * std::cos(ry) * cosX
    );
    
    QVector3D target = modelCenter + QVector3D(panOffset.x(), panOffset.y(), 0.0f);
    QVector3D up = (cosX >= 0.0f) ? QVector3D(0,1,0) : QVector3D(0,-1,0);
    
    QMatrix4x4 view;
    view.lookAt(camPos + target, target, up);
    QMatrix4x4 mvp = proj * view;
    
    // �������ж��㣬�ҵ���Ļ���������
    int bestVertex = -1;
float bestDistance = 15.0f; // ������ֵ��ֻʰȡ15�����ڵĶ���
  
    for (int i = 0; i < (int)vertices.size(); ++i) {
        // ����任���ü��ռ�
        QVector4D clipPos = mvp * QVector4D(vertices[i], 1.0f);
        
        if (clipPos.w() == 0) continue; // �������
        
        // ͸�ӳ������õ�NDC����[-1,1]
      QVector3D ndc(clipPos.x() / clipPos.w(), 
               clipPos.y() / clipPos.w(), 
          clipPos.z() / clipPos.w());
     
        // NDCת��Ļ����
        QPoint screenPos(
  (int)((ndc.x() * 0.5f + 0.5f) * width()),
     (int)((-ndc.y() * 0.5f + 0.5f) * height())
        );
        
        // ������Ļ����
        float dist = std::hypot((float)(screenPos.x() - pos.x()), 
      (float)(screenPos.y() - pos.y()));
        
     if (dist < bestDistance) {
            bestDistance = dist;
      bestVertex = i;
        }
    }
    
    if (bestVertex >= 0) {
std::cout << "[GLWidget] Picked vertex " << bestVertex 
    << " at screen distance " << bestDistance << " pixels" << std::endl;
    }
    
    return bestVertex;
}

/**
 * @brief ��Ļ����ת����3D����
 * @param pos ��Ļ���꣨���λ�ã�
 * @param depth ���ֵ����������ľ��룬�ö���ԭʼ��ȣ�
 * @return 3D��������
 * 
 * ʵ��ԭ��
 * 1. ����Ļ�����һ����NDC�ռ�[-1,1]
 * 2. ������ƽ���Զƽ���ϵ�������
 * 3. ��任������ռ�
 * 4. �����߷����ƶ���ָ�����
 * 
 * ��;���������ק��2D�켣ת��Ϊhandle�����3D��λ��
 */
QVector3D GLWidget::screenToWorld(const QPoint& pos, float depth) const {
    // ����MVP����
    float aspect = (width() > 0 && height() > 0) ? (float)width() / (float)height() : 1.0f;
    QMatrix4x4 proj;
    proj.perspective(45.0f, aspect, 0.01f, 1000.0f);
    
    float rx = qDegreesToRadians(rotationX);
  float ry = qDegreesToRadians(rotationY);
  float cosX = std::cos(rx);
    
    QVector3D camPos(
    distance * std::sin(ry) * cosX,
        distance * std::sin(rx),
        distance * std::cos(ry) * cosX
    );
    
 QVector3D target = modelCenter + QVector3D(panOffset.x(), panOffset.y(), 0.0f);
    QVector3D up = (cosX >= 0.0f) ? QVector3D(0,1,0) : QVector3D(0,-1,0);
    
    QMatrix4x4 view;
    view.lookAt(camPos + target, target, up);
    
    // ������MVP����
    QMatrix4x4 invMVP = (proj * view).inverted();
    
    // ��Ļ����תNDC
    float nx = (2.0f * pos.x() / (float)width()) - 1.0f;
    float ny = 1.0f - (2.0f * pos.y() / (float)height());
    
    // ������ƽ���Զƽ���ϵĵ�
    QVector4D nearPoint = invMVP * QVector4D(nx, ny, -1.0f, 1.0f);
    QVector4D farPoint = invMVP * QVector4D(nx, ny, 1.0f, 1.0f);
    
    // ͸�ӳ���
    nearPoint /= nearPoint.w();
    farPoint /= farPoint.w();
    
    // �������߷���
    QVector3D rayDir = (farPoint - nearPoint).toVector3D().normalized();
    QVector3D rayOrigin = nearPoint.toVector3D();
    
    // �������ƶ���ָ�����
    return rayOrigin + rayDir * depth;
}

/**
 * @brief ִ��ARAP��ק����
 * @param pos ��ǰ�����Ļλ��
 * 
 * �������̣�
 * 1. �����λ��ת��Ϊ3D���꣨����handleԭʼ��ȣ�
 * 2. ����arapDragCallbackִ��ARAP�㷨
 * 3. �÷��ص���mesh������ʾ�������¶��㻺�壬��������ͼ��
 */
void GLWidget::performArapDrag(const QPoint& pos) {
    if (arapHandleVertex < 0 || !arapDragCallback) {
        return;
    }
    
    // ����Ļ����ת��Ϊ3D��������
    QVector3D newWorldPos = screenToWorld(pos, handleDepth);
    
 // ����ARAP�㷨�ص�
    auto result = arapDragCallback(arapHandleVertex, newWorldPos);
    
    // ֻ���¶������ݣ���������ͼ�����¼����Χ��
    vertices = result.first;
    indices = result.second;
    
    if (!isValid()) return;
    
    // ֻ���� GPU ������
    vertexBuf.bind();
    vertexBuf.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(QVector3D)));
    
    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));
    
    // ������ calculateModelBounds() �� resetView()
    update();  // �������ػ�
}

/**
 * @brief ����ARAPѡ��ģʽ
 */
void GLWidget::setArapSelectionMode(ArapSelectionMode mode) {
 arapSelectionMode = mode;
 const char* modeNames[] = {"None", "SelectFixed", "SelectHandle"};
 std::cout << "[GLWidget] ARAP selection mode: " << modeNames[static_cast<int>(mode)] << std::endl;
 update();
}