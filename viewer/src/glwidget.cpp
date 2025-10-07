#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QDebug>
#include <QtCore/QResource>
#include <QFile>
#include <cmath>
#include <limits>

/* --------------------------------------------------------------------------
 * GLWidget
 * 负责:
 *  - 载入与存储网格顶点/索引/颜色数据
 *  - OpenGL 上下文初始化与着色器构建
 *  - 相机/交互(旋转、缩放)
 *  - 绘制主网格(线框) + MST 高亮线段
 * -------------------------------------------------------------------------- */

GLWidget::GLWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      distance(4.0f),
      rotationX(0.0f),
      rotationY(0.0f),
      modelOffsetY(0.0f) {
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

/* ---------------------------- 数据更新: 主网格 ---------------------------- */
void GLWidget::updateMesh(const std::vector<QVector3D>& v,
                          const std::vector<unsigned int>& idx) {
    vertices = v;
    indices  = idx;
    perVertexColor = false; // 默认不使用外部颜色
    colors.assign(vertices.size(), QVector3D(1.0f, 1.0f, 1.0f));

    if (!isValid()) return; // OpenGL 尚未初始化

    vertexBuf.bind();
    vertexBuf.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(QVector3D)));

    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));

    colorBuf.bind();
    colorBuf.allocate(colors.data(), static_cast<int>(colors.size() * sizeof(QVector3D)));

    calculateModelBounds();
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
        mstLineBuf.allocate(nullptr, 0); // 释放显存
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 使用线框模式

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

    // 构建 MVP 矩阵
    float aspect = (width() > 0 && height() > 0) ? (float)width() / (float)height() : 1.0f;
    QMatrix4x4 proj;  proj.perspective(45.0f, aspect, 0.01f, 100.0f);
    QMatrix4x4 view;  view.translate(0.0f, 0.0f, -distance);
    QMatrix4x4 model; model.translate(0.0f, modelOffsetY, 0.0f);
    model.rotate(rotationX, 1.0f, 0.0f, 0.0f);
    model.rotate(rotationY, 0.0f, 1.0f, 0.0f);
    QMatrix4x4 mvp = proj * view * model;

    program->bind();
    program->setUniformValue("u_mvp", mvp);

    // 主网格
    vertexBuf.bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));

    colorBuf.bind();
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(QVector3D));

    indexBuf.bind();
    glDrawElements(GL_TRIANGLES, static_cast<int>(indices.size()), GL_UNSIGNED_INT, nullptr);

    // 第二遍：绘制 MST 红色线段
    if (!mstLineVertices.empty()) {
        glLineWidth(3.0f);
        program->disableAttributeArray(1);          // 不使用颜色缓冲
        glVertexAttrib3f(1, 1.0f, 0.0f, 0.0f);       // 固定红色
        mstLineBuf.bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        glDrawArrays(GL_LINES, 0, static_cast<int>(mstLineVertices.size()));
        glLineWidth(1.0f);
        // 恢复颜色缓冲
        colorBuf.bind();
        program->enableAttributeArray(1);
        program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(QVector3D));
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

    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = std::max({ sizeX, sizeY, sizeZ });

    // 小模型统一放大
    if (maxSize < 1.0f && maxSize > 0.0f) {
        float scale = 2.0f / maxSize;
        for (auto& v : vertices) {
            const_cast<QVector3D&>(v) = v * scale;
        }
        maxSize *= scale;
        minY    *= scale;
    }

    distance     = std::max(2.0f, std::min(maxSize * 2.0f, 50.0f));
    modelOffsetY = -minY - 0.2f;
}

/* ------------------------------- 载入 OBJ 文件 ---------------------------- */
bool GLWidget::loadObject(const QString& file) {
    if (objLoader.loadOBJ(file.toStdString())) {
        updateMesh(objLoader.vertices, objLoader.indices);
        mstLineVertices.clear(); // 清除旧的 MST 线
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
}

void GLWidget::mouseMoveEvent(QMouseEvent* e) {
    int dx = e->x() - lastPos.x();
    int dy = e->y() - lastPos.y();
    if (e->buttons() & Qt::LeftButton) {
        rotationY += dx * 0.5f;
        rotationX += dy * 0.5f;
        update();
    }
    lastPos = e->pos();
}

void GLWidget::wheelEvent(QWheelEvent* e) {
    float delta = e->angleDelta().y() / 120.0f; // 每个滚轮刻度 120
    distance -= delta * 0.6f;
    distance = std::clamp(distance, 1.0f, 80.0f);
    update();
}