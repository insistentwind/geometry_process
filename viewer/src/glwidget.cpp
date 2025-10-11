#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QDebug>
#include <QtCore/QResource>
#include <QFile>
#include <cmath>
#include <limits>

/* --------------------------------------------------------------------------
 * GLWidget 改进版: 添加轨道相机(中心化, 平移, 更平滑缩放)
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

/* ---------------------------- 数据更新: 主网格 ---------------------------- */
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

/* ---------------------------- 数据更新: 带颜色 ----------------------------- */
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

/* ------------------------------ 更新 MST 线段 ----------------------------- */
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

/* ------------------------------ 清除 MST 高亮 ------------------------------ */
void GLWidget::clearMSTEdges() {
    mstLineVertices.clear();
    if (mstLineBuf.isCreated()) {
        mstLineBuf.bind();
        mstLineBuf.allocate(nullptr, 0);
    }
    update();
}

/* ------------------------------- 访问接口 ------------------------------- */
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

/* ----------------------------- OpenGL 初始化 ------------------------------ */
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

/* --------------------------------- 绘制 ---------------------------------- */
void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (vertices.empty() || !program || !program->isLinked()) return;

    // 线框模式始终保持，用于轮廓；填充面可选叠加
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

    // 先可选绘制填充面 (GL_FILL)，再绘制线框 (GL_LINE) 覆盖提高可见性
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
            glVertexAttrib3f(1, 0.85f, 0.85f, 0.85f); // 默认淡灰
        }
        indexBuf.bind();
        glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    // 再绘制线框
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    vertexBuf.bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
    program->disableAttributeArray(1);
    glVertexAttrib3f(1, 1.0f, 1.0f, 1.0f);
    indexBuf.bind();
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);

    // MST 线
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

    // 点云
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

    program->disableAttributeArray(0);
    program->disableAttributeArray(1);
    program->release();
}

/* ------------------------------- 计算包围盒 ------------------------------- */
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
    distance = modelRadius * 2.2f;
    distance = std::clamp(distance, 0.5f * modelRadius, 10.0f * modelRadius);
}

/* ------------------------------- 载入 OBJ 文件 ---------------------------- */
bool GLWidget::loadObject(const QString& file) {
    if (objLoader.loadOBJ(file.toStdString())) {
        updateMesh(objLoader.vertices, objLoader.indices);
        mstLineVertices.clear();
        return true;
    }
    return false;
}

/* ------------------------------- 窗口尺寸变化 ----------------------------- */
void GLWidget::resizeGL(int w, int h) {
    projection.setToIdentity();
    if (h > 0) {
        projection.perspective(45.0f, static_cast<float>(w) / static_cast<float>(h), 0.01f, 100.0f);
    }
}

/* --------------------------------- 交互事件 -------------------------------- */
void GLWidget::mousePressEvent(QMouseEvent* e) {
    lastPos = e->pos();
    if (e->button() == Qt::RightButton) {
        resetView();
        update();
    }
}

void GLWidget::mouseMoveEvent(QMouseEvent* e) {
    int dx = e->x() - lastPos.x();
    int dy = e->y() - lastPos.y();

    if (e->buttons() & Qt::LeftButton) {
        rotationY -= dx * 0.5f;
        rotationX += dy * 0.5f;
        if (rotationX > 179.f) rotationX -= 360.f;
        if (rotationX < -179.f) rotationX += 360.f;
        update();
    } else if (e->buttons() & Qt::MiddleButton) {
        float panScale = distance * 0.0015f;
        panOffset += QVector2D(-dx * panScale, dy * panScale);
        update();
    }

    lastPos = e->pos();
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