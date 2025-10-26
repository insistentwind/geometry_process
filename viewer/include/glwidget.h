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
#include <set>
#include <functional>

/**
 * @brief OpenGL渲染窗口类，支持网格显示和ARAP交互
 * 
 * 新增ARAP功能：
 * - 进入/退出ARAP模式
 * - 鼠标点击拾取顶点（设置fixed点）
 * - 鼠标拖拽顶点（作为handle进行变形）
 */
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    enum class ColorDisplayMode { PointsOnly, Faces };
    
    // ==================== ARAP选择模式 ====================
    /**
     * @brief ARAP选择模式枚举
     * SelectFixed: 选择固定点模式（可多选）
     * SelectHandle: 选择拖动点模式（单选）
     * None: 不选择，可以拖动视图
     */
    enum class ArapSelectionMode {
     None,          // 正常模式（可旋转视图）
        SelectFixed,   // 选择Fixed点模式
        SelectHandle   // 选择Handle点模式
    };

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
    void toggleFilledFaces() { showFilledFaces = !showFilledFaces; update(); }
    bool isShowingFilledFaces() const { return showFilledFaces; }

    // ==================== ARAP交互API ====================
    /**
     * @brief 切换ARAP模式开关
     * 作用：进入ARAP模式后，左键点击选择fixed顶点，拖拽实现变形
     */
    void toggleArapMode();
    
    /**
     * @brief 查询当前是否处于ARAP模式
     */
    bool isArapActive() const { return arapActive; }
    
    /**
     * @brief 清除所有已选择的fixed顶点
     * 作用：清空固定点约束
     */
    void clearArapFixedVertices();
    
    /**
     * @brief 设置ARAP选择模式
     * @param mode 选择模式（None/SelectFixed/SelectHandle）
     */
    void setArapSelectionMode(ArapSelectionMode mode);
    
    /**
     * @brief 获取当前ARAP选择模式
     */
    ArapSelectionMode getArapSelectionMode() const { return arapSelectionMode; }

    // ==================== ARAP回调函数（在hw6.cpp中设置） ====================
    /**
     * @brief ARAP会话开始回调
     * 外部设置此函数来初始化ARAP算法（保存old_position等）
     * 调用时机：进入ARAP模式时
     */
    std::function<void()> arapBeginCallback;
    
    /**
* @brief 设置固定顶点回调
     * @param int 顶点索引
     * @param bool 是否固定
     * 外部设置此函数来标记哪些顶点是fixed的
     * 调用时机：用户点击顶点时
     */
    std::function<void(int, bool)> arapSetFixedCallback;
    
    /**
     * @brief 清除所有fixed顶点回调
     * 外部设置此函数来清空所有固定点标记
     * 调用时机：用户点击"clear fixed"按钮时
     */
    std::function<void()> arapClearFixedCallback;
    
    /**
     * @brief ARAP拖拽变形回调
     * @param int handle顶点索引
     * @param QVector3D handle的新位置
     * @return 变形后的网格数据（顶点和索引）
     * 外部设置此函数来执行ARAP算法并返回新mesh
     * 调用时机：用户拖拽顶点时，实时调用
     */
    std::function<std::pair<std::vector<QVector3D>, std::vector<unsigned int>>(int, const QVector3D&)> arapDragCallback;

protected:
    void initializeGL() override;
  void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
  void calculateModelBounds();
    void resetView();
    
 // ==================== ARAP辅助函数 ====================
    /**
     * @brief 屏幕坐标拾取最近的顶点
     * @param pos 屏幕坐标（鼠标位置）
     * @return 顶点索引，-1表示未拾取到
   * 
     * 实现原理：
     * 1. 将所有顶点投影到屏幕空间
     * 2. 计算鼠标位置与每个顶点屏幕位置的距离
     * 3. 返回距离最近且在阈值内的顶点
     */
    int pickVertex(const QPoint& pos) const;
    
    /**
     * @brief 屏幕坐标转换为世界3D坐标
     * @param pos 屏幕坐标
     * @param depth 深度值（距离相机的距离）
   * @return 世界坐标
     * 
     * 用途：将鼠标拖拽的2D位置转换为3D空间中handle点的新位置
     */
    QVector3D screenToWorld(const QPoint& pos, float depth) const;
    
    /**
  * @brief 执行ARAP拖拽变形
  * @param pos 当前鼠标屏幕位置
     * 
     * 工作流程：
     * 1. 将鼠标位置转换为3D坐标（保持原深度）
     * 2. 调用arapDragCallback执行ARAP算法
     * 3. 更新mesh显示
     */
    void performArapDrag(const QPoint& pos);
    
    /**
     * @brief 开始ARAP会话（如果需要）
   * 调用arapBeginCallback初始化
     */
    void beginArapIfNeeded();

    // 原有成员变量
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
    float pointSize = 3.0f;  // 改为3.0f使点更小

    ColorDisplayMode colorMode { ColorDisplayMode::PointsOnly };
    bool showFilledFaces = false;
    
    // ==================== ARAP状态变量 ====================
    bool arapActive = false;     // 是否在ARAP模式
    ArapSelectionMode arapSelectionMode = ArapSelectionMode::None; // ARAP选择模式
    int arapHandleVertex = -1; // 当前拖拽的handle顶点索引（-1表示未选择）
    std::set<int> fixedVertices; // 所有标记为fixed的顶点索引集合
    bool leftButtonDraggingHandle = false;// 是否正在拖拽handle
    float handleDepth = 0.0f;     // handle顶点的初始深度（用于屏幕转世界坐标）
};

#endif // GLWIDGET_H