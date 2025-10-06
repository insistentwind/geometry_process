#include "glwidget.h"
#include <QMouseEvent>
#include <cmath>
#include <limits>

/*
 * GLWidget.cpp 详解
 * ------------------
 * 该文件实现了基于旧式固定管线(OpenGL 1.x 风格) 的一个简单 3D 模型查看组件。
 * 主要职责：
 *   1. 管理模型几何数据(通过内部的 ObjLoader 保存顶点与索引)。
 *   2. 根据窗口尺寸设置投影矩阵 (使用 glFrustum 模拟透视投影)。
 *   3. 在 paintGL 中执行模型视图矩阵堆栈操作：重置 -> 平移(拉远“相机”) -> 旋转 -> Y 轴校正偏移。
 *   4. 对模型进行简单的自适应缩放与居中/底部对齐处理 (calculateModelBounds)。
 *   5. 提供基础交互：
 *        - 鼠标左键拖动：绕 X/Y 轴旋转模型
 *        - 鼠标滚轮：改变 distance，实现缩放 (本质是沿 Z 轴平移)
 *   6. 支持外部调用 updateMesh 替换当前显示的几何数据(例如经过算法处理后的平滑模型)。
 *
 * 重要成员含义：
 *   - distance:      场景沿 Z 负方向平移的距离，模拟“相机后移”。
 *   - rotationX/Y:   交互产生的模型旋转角度 (绕 X / Y 轴)。
 *   - modelOffsetY:  Y 方向额外平移，使模型底面与视图下方对齐，改善视觉体验。
 *   - objLoader:     存放加载的 .obj 顶点(vertex) 与索引(indices)。
 *
 * 典型渲染流程 (paintGL):
 *   1. 清屏 (颜色 + 深度)
 *   2. 设置投影矩阵 (glFrustum)
 *   3. 重置模型视图矩阵 (glLoadIdentity)
 *   4. glTranslatef(0,0,-distance) 把“相机”拉远
 *   5. glRotatef 叠加交互旋转
 *   6. glTranslatef(0, modelOffsetY, 0) 让模型底部上移到可视区域底部
 *   7. 线框模式 + 白色
 *   8. glBegin(GL_TRIANGLES) + 遍历索引绘制
 *
 * 注意：本实现使用了固定功能管线 (Immediate Mode)，现代 OpenGL 推荐使用 VAO/VBO + 着色器。
 * 在教学 / 轻量快速验证场景中仍可使用这种方式，但不适用于高性能或可扩展渲染系统。
 */

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      distance(1.0f),        // 初始“相机”拉远距离
      rotationX(0.0f),       // 绕 X 轴初始旋转角
      rotationY(0.0f),       // 绕 Y 轴初始旋转角
      modelOffsetY(0.0f)     // 模型 Y 方向偏移初始化
{
    // 允许该部件获取键盘焦点及鼠标事件
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    // 确保在当前 OpenGL 上下文中销毁缓冲对象 (虽然当前代码未真正使用 VBO/VAO 绘制)
    makeCurrent();
    vbo.destroy();
    vao.destroy();
    doneCurrent();
}

void GLWidget::initializeGL()
{
    // 初始化 OpenGL 函数指针 (由 QOpenGLFunctions 提供)
    initializeOpenGLFunctions();

    // 基础状态：背景色 + 深度测试
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GLWidget::paintGL()
{
    // 1. 清除颜色与深度缓冲，准备新一帧
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 没有顶点数据就不绘制
    if (!objLoader.vertices.empty()) {
        // 2. ---- 设置投影矩阵 ----
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity(); // 重置投影矩阵
        float aspect = width() / (float)height();
        // 使用对称视锥体，near=1, far=100；aspect 控制左右范围
        glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 100.0);

        // 3. ---- 设置模型视图矩阵 ----
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity(); // 重置模型视图矩阵
        // 将场景整体沿Z轴负方向平移 distance，相当于把“相机”往后拉开，确保模型处在可视范围内
        glTranslatef(0.0f, 0.0f, -distance);
        // 旋转顺序：先绕 X，再绕 Y，产生轨迹球般的旋转体验
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);
        
        // 额外 Y 轴偏移：将模型底部移动到屏幕较低位置，避免居中显得“漂浮”
        glTranslatef(0.0f, modelOffsetY, 0.0f);

        // 4. 线框渲染便于查看拓扑结构
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(1.0f, 1.0f, 1.0f); // 白色线框

        // 5. Immediate Mode 绘制三角形
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
    
    // 遍历所有顶点，计算 AABB (轴对齐包围盒)
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
    
    // 计算整体尺寸，用于估计“相机”距离，以及必要时的缩放
    float sizeX = maxX - minX;
    float sizeY = maxY - minY;
    float sizeZ = maxZ - minZ;
    float maxSize = std::max({sizeX, sizeY, sizeZ});
    
    // 若模型尺度过小，统一放大到近似大小 2.0 的范围，避免显示过小
    if (maxSize < 1.0f) {
        float scaleFactor = 2.0f / maxSize; // 目标范围 ~2.0
        for (auto& vertex : objLoader.vertices) {
            vertex = vertex * scaleFactor;
        }
        // 由于直接缩放了原数据，这里同步缩放边界值 (避免再次遍历)
        minX *= scaleFactor; maxX *= scaleFactor;
        minY *= scaleFactor; maxY *= scaleFactor;
        minZ *= scaleFactor; maxZ *= scaleFactor;
        maxSize *= scaleFactor;
    }
    
    // 根据最大尺度估算距离，使模型大致占据视野
    distance = maxSize * 2.0f; // 经验系数
    if (distance < 0.0f)  distance = 1.0f;   // 下限防止穿近裁剪面
    if (distance > 100.0f) distance = 100.0f; // 上限避免过远
    
    // 计算 Y 轴偏移，使底部位置接近视锥体下方 (-0.8 ~ 经验值)
    modelOffsetY = -minY - 0.8f;
    
    // 加载/更新后重置交互旋转
    rotationX = 0.0f;
    rotationY = 0.0f;
}

// 加载 OBJ 文件：成功则计算包围盒并请求重绘
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
    // 这里仅更新一个 QMatrix4x4 形式的投影矩阵 (当前渲染固定管线未直接使用)
    float aspect = width / (float)height;
    projection.setToIdentity();
    projection.perspective(45.0f, aspect, 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    // 记录初始点击位置，用于之后的拖动计算增量
    lastPos = event->pos();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x(); // 水平位移
    int dy = event->y() - lastPos.y(); // 垂直位移

    if (event->buttons() & Qt::LeftButton) {
        // 简单映射：水平拖动 -> Y 轴旋转；垂直拖动 -> X 轴旋转
        rotationY += dx;
        rotationX += dy;
        update(); // 请求重绘
    }
    lastPos = event->pos();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    // angleDelta().y() 每个刻度 120，换算为缩放步进
    float delta = event->angleDelta().y() / 120.0f;
    distance -= delta * 0.5f; // 向前滚轮 -> 拉近
    // 限制范围，避免穿近裁剪面或过远
    distance = std::max(2.0f, std::min(distance, 20.0f));
    update();
}

void GLWidget::updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices)
{
    // 替换当前模型数据 (通常来自算法处理结果)
    objLoader.vertices = vertices;
    objLoader.indices  = indices;
    calculateModelBounds(); // 重新计算尺寸与偏移
    update();               // 请求重绘
}