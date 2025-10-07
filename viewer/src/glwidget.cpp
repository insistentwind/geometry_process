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
/*
* 这里是根据顶点idx来绘制曲面？
*/

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
    // 1. 复制/接收外部传入的网格几何数据到本地缓存（CPU 侧保存一份，方便后续再上传或做查询）
    vertices = v;          // 顶点位置列表
    indices  = idx;        // 三角面索引 (按 3 个一组构成三角形)

    // 2. 处理颜色数据: 若传入颜色数组尺寸与顶点一致则使用；否则退化为全白
    if (cols.size() == v.size()) {
        colors = cols;     // 使用外部 per-vertex 颜色
        perVertexColor = true;  // 标记启用自定义颜色（可用于渲染策略判断）
    } else {
        // 尺寸不匹配 -> 填充为白色，避免越界/未定义访问
        colors.assign(v.size(), QVector3D(1.0f, 1.0f, 1.0f));
        perVertexColor = false;
    }

    // 3. 若此时 OpenGL 上下文还不可用（窗口尚未初始化完成），提前返回；
    //    数据仍已保存于 CPU，稍后 initializeGL 后你可以再次调用或触发一次重上传
    if (!isValid()) return;

    // 4. 上传顶点位置到顶点缓冲 (VBO)
    //    allocate 会重新分配并复制数据（当前使用 DynamicDraw，允许后续多次更新）
    vertexBuf.bind();
    vertexBuf.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(QVector3D)));

    // 5. 上传索引 (EBO)，供 glDrawElements 按三角形装配
    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));

    // 6. 上传颜色缓冲，与位置一一对应
    colorBuf.bind();
    colorBuf.allocate(colors.data(), static_cast<int>(colors.size() * sizeof(QVector3D)));

    // 7. 重新计算模型包围盒 & 视距等（可能模型尺寸变化）
    calculateModelBounds();

    // 8. 请求窗口重绘（Qt 事件循环稍后调用 paintGL）
    update();
}

/* ------------------------------ 更新 MST 线段 ----------------------------- */
void GLWidget::updateMSTEdges(const std::vector<std::pair<int, int>>& edges) {
    // 1. 清空旧的 MST 线段顶点缓存（CPU 侧）
    mstLineVertices.clear();
    // 2. 预分配容量（每条无向边转成两个端点）以减少 vector 扩容开销
    mstLineVertices.reserve(edges.size() * 2);

    // 3. 将 (i,j) 顶点索引对转换为实际 3D 坐标并追加
    for (const auto& e : edges) {
        int a = e.first;
        int b = e.second;
        // 基础合法性检查：索引在当前顶点数组范围内
        if (a >= 0 && b >= 0 && a < static_cast<int>(vertices.size()) && b < static_cast<int>(vertices.size())) {
            // 直接两次 push_back 构成一条线段的两个端点（GL_LINES）
            mstLineVertices.push_back(vertices[a]);
            mstLineVertices.push_back(vertices[b]);
        }
        // 若非法则忽略该边（不抛异常，保证稳健性）
    }

    // 4. 若显存中的 MST 线段缓冲还未创建，先创建
    if (!mstLineBuf.isCreated()) {
		mstLineBuf.create();// 线段缓冲是为了绘制红色线段
    }
    // 5. 绑定并上传新线段数据（即使为空也会正确处理）
    mstLineBuf.bind();
    mstLineBuf.allocate(mstLineVertices.data(), static_cast<int>(mstLineVertices.size() * sizeof(QVector3D)));

    // 6. 请求重绘：下一帧 paintGL 中会检测 mstLineVertices 是否为空来决定是否绘制红线
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