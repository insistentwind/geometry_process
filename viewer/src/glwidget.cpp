#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QDebug>
#include <QtCore/QResource>
#include <QFile>
#include <cmath>
#include <limits>
#include <iostream> // 添加iostream用于std::cout

/* --------------------------------------------------------------------------
 * GLWidget 改进版: 添加轨道相机(中心化, 平移, 更平滑缩放) + ARAP交互
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

    // ARAP模式：高亮显示所有fixed顶点（蓝色大点）
    if (arapActive && !fixedVertices.empty()) {
    glPointSize(12.0f);  // 较大的点
        vertexBuf.bind();
      program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        program->disableAttributeArray(1);
glVertexAttrib3f(1, 0.2f, 0.5f, 1.0f);  // 蓝色

        // 绘制所有fixed顶点
        for (int vertexIndex : fixedVertices) {
            if (vertexIndex >= 0 && vertexIndex < static_cast<int>(vertices.size())) {
   glDrawArrays(GL_POINTS, vertexIndex, 1);
     }
        }
    }
    
    // ARAP模式，绘制handle顶点（红色点）
    if (arapActive && arapHandleVertex >= 0) {
   glPointSize(15.0f);  // 更大的点
  vertexBuf.bind();
        program->enableAttributeArray(0);
        program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));
        program->disableAttributeArray(1);
        glVertexAttrib3f(1, 1.0f, 0.2f, 0.2f);  // 红色
        
 glDrawArrays(GL_POINTS, arapHandleVertex, 1);
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
    distance = modelRadius * 3.5f;  // 增加到3.5倍，使模型显示更小
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

 // ARAP模式下的鼠标操作：根据选择模式决定行为
 if (arapActive && e->button() == Qt::LeftButton) {
 int hitVertex = pickVertex(e->pos());
 if (hitVertex >=0) {
 if (arapSelectionMode == ArapSelectionMode::SelectFixed) {
 // === Fixed点选择模式：添加到fixed集合 ===
 fixedVertices.insert(hitVertex);
 // 调用回调标记为fixed
 if (arapSetFixedCallback) {
 arapSetFixedCallback(hitVertex, true);
 std::cout << "[GLWidget] Marked vertex " << hitVertex << " as FIXED (total: "
 << fixedVertices.size() << " fixed vertices)" << std::endl;
 }
 }
 else if (arapSelectionMode == ArapSelectionMode::SelectHandle) {
 // === Handle点选择模式：设置为拖动点 ===
 arapHandleVertex = hitVertex;
 //计算相机位置
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
 //记录handle深度（距离相机）
 handleDepth = (vertices[hitVertex] - camPos).length();
 //允许后续 mouseMoveEvent触发拖拽
 leftButtonDraggingHandle = true;
 std::cout << "[GLWidget] Selected vertex " << hitVertex << " as HANDLE (ready to drag)" << std::endl;
 }
 }
 return; // ARAP模式下，左键不旋转视图
 }
 //右键重置视图
 if (e->button() == Qt::RightButton) {
 resetView();
 update();
 }
}

void GLWidget::mouseMoveEvent(QMouseEvent* e) {
 int dx = e->x() - lastPos.x();
 int dy = e->y() - lastPos.y();

 if (arapActive) {
 // 若已选定 handle 且左键仍按下则执行拖拽
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

/* ==================== ARAP交互功能实现 ==================== */

/**
 * @brief 切换ARAP模式
 * 
 * 进入ARAP模式时：
 * - 调用arapBeginCallback初始化（保存old_position）
 * - 左键点击选择fixed顶点
 * - 左键拖拽执行ARAP变形
 * 
 * 退出ARAP模式时：
 * - 重置handle顶点索引
 * - 恢复正常相机交互
 */
void GLWidget::toggleArapMode() {
  arapActive = !arapActive;
    
    if (!arapActive) {
        // 退出ARAP模式，清理状态
    fixedVertices.clear();
      arapHandleVertex = -1;
      leftButtonDraggingHandle = false;
        arapSelectionMode = ArapSelectionMode::None; // 重置选择模式
  std::cout << "[GLWidget] Exited ARAP mode" << std::endl;
    } else {
      // 进入ARAP模式，初始化（只调用一次）
     if (arapBeginCallback) {
    arapBeginCallback();
        }
        arapSelectionMode = ArapSelectionMode::SelectFixed; // 默认进入Fixed选择模式
    std::cout << "[GLWidget] Entered ARAP mode - Use toolbar buttons to select Fixed/Handle vertices" << std::endl;
    }

    update();
}

/**
 * @brief 清除所有已选择的固定顶点
 */
void GLWidget::clearArapFixedVertices() {
    if (arapClearFixedCallback) {
        arapClearFixedCallback();
    }
    
    // 清空fixed顶点集合
    fixedVertices.clear();
    
// 重置handle状态
    arapHandleVertex = -1;
    leftButtonDraggingHandle = false;
    
    std::cout << "[GLWidget] Cleared all fixed vertices" << std::endl;
    update();
}

/**
 * @brief 开始ARAP会话（如果回调已设置）
 */
void GLWidget::beginArapIfNeeded() {
    if (arapBeginCallback) {
    arapBeginCallback();
        std::cout << "[GLWidget] ARAP session initialized" << std::endl;
    }
}

/**
 * @brief 屏幕坐标拾取最近的顶点
 * @param pos 鼠标屏幕坐标
 * @return 顶点索引，-1表示未拾取到
 * 
 * 实现原理：
 * 1. 构建MVP矩阵（模型-视图-投影矩阵）
 * 2. 将所有顶点变换到屏幕空间
 * 3. 计算鼠标位置与每个顶点的屏幕距离
 * 4. 返回距离最近且在15像素阈值内的顶点
 */
int GLWidget::pickVertex(const QPoint& pos) const {
    if (vertices.empty()) return -1;
    
    // 构建MVP矩阵（与paintGL中相同）
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
    
    // 遍历所有顶点，找到屏幕距离最近的
    int bestVertex = -1;
float bestDistance = 15.0f; // 像素阈值：只拾取15像素内的顶点
  
    for (int i = 0; i < (int)vertices.size(); ++i) {
        // 顶点变换到裁剪空间
        QVector4D clipPos = mvp * QVector4D(vertices[i], 1.0f);
        
        if (clipPos.w() == 0) continue; // 避免除零
        
        // 透视除法，得到NDC坐标[-1,1]
      QVector3D ndc(clipPos.x() / clipPos.w(), 
               clipPos.y() / clipPos.w(), 
          clipPos.z() / clipPos.w());
     
        // NDC转屏幕坐标
        QPoint screenPos(
  (int)((ndc.x() * 0.5f + 0.5f) * width()),
     (int)((-ndc.y() * 0.5f + 0.5f) * height())
        );
        
        // 计算屏幕距离
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
 * @brief 屏幕坐标转世界3D坐标
 * @param pos 屏幕坐标（鼠标位置）
 * @param depth 深度值（距离相机的距离，用顶点原始深度）
 * @return 3D世界坐标
 * 
 * 实现原理：
 * 1. 将屏幕坐标归一化到NDC空间[-1,1]
 * 2. 构建近平面和远平面上的两个点
 * 3. 逆变换到世界空间
 * 4. 沿视线方向移动到指定深度
 * 
 * 用途：将鼠标拖拽的2D轨迹转换为handle顶点的3D新位置
 */
QVector3D GLWidget::screenToWorld(const QPoint& pos, float depth) const {
    // 构建MVP矩阵
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
    
    // 计算逆MVP矩阵
    QMatrix4x4 invMVP = (proj * view).inverted();
    
    // 屏幕坐标转NDC
    float nx = (2.0f * pos.x() / (float)width()) - 1.0f;
    float ny = 1.0f - (2.0f * pos.y() / (float)height());
    
    // 构建近平面和远平面上的点
    QVector4D nearPoint = invMVP * QVector4D(nx, ny, -1.0f, 1.0f);
    QVector4D farPoint = invMVP * QVector4D(nx, ny, 1.0f, 1.0f);
    
    // 透视除法
    nearPoint /= nearPoint.w();
    farPoint /= farPoint.w();
    
    // 计算视线方向
    QVector3D rayDir = (farPoint - nearPoint).toVector3D().normalized();
    QVector3D rayOrigin = nearPoint.toVector3D();
    
    // 沿视线移动到指定深度
    return rayOrigin + rayDir * depth;
}

/**
 * @brief 执行ARAP拖拽变形
 * @param pos 当前鼠标屏幕位置
 * 
 * 工作流程：
 * 1. 将鼠标位置转换为3D坐标（保持handle原始深度）
 * 2. 调用arapDragCallback执行ARAP算法
 * 3. 用返回的新mesh更新显示（仅更新顶点缓冲，不重置视图）
 */
void GLWidget::performArapDrag(const QPoint& pos) {
    if (arapHandleVertex < 0 || !arapDragCallback) {
        return;
    }
    
    // 将屏幕坐标转换为3D世界坐标
    QVector3D newWorldPos = screenToWorld(pos, handleDepth);
    
 // 调用ARAP算法回调
    auto result = arapDragCallback(arapHandleVertex, newWorldPos);
    
    // 只更新顶点数据，不重置视图或重新计算包围盒
    vertices = result.first;
    indices = result.second;
    
    if (!isValid()) return;
    
    // 只更新 GPU 缓冲区
    vertexBuf.bind();
    vertexBuf.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(QVector3D)));
    
    indexBuf.bind();
    indexBuf.allocate(indices.data(), static_cast<int>(indices.size() * sizeof(unsigned int)));
    
    // 不调用 calculateModelBounds() 和 resetView()
    update();  // 仅触发重绘
}

/**
 * @brief 设置ARAP选择模式
 */
void GLWidget::setArapSelectionMode(ArapSelectionMode mode) {
 arapSelectionMode = mode;
 const char* modeNames[] = {"None", "SelectFixed", "SelectHandle"};
 std::cout << "[GLWidget] ARAP selection mode: " << modeNames[static_cast<int>(mode)] << std::endl;
 update();
}