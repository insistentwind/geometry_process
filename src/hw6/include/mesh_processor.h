#pragma once

#include <QVector3D>
#include <vector>
#include <utility>
#include <halfedge.h>

/**
 * @brief 网格处理类，用于将OBJ数据转换为半边结构并进行几何处理
 * 负责调用geometry模块的转换功能和实现具体的几何处理算法
 * 与UI完全解耦，专注于网格处理算法
 */
class MeshProcessor {
public:
    /**
     * @brief 构造函数
     */
    MeshProcessor() = default;

    /**
     * @brief 析构函数
     */
    ~MeshProcessor() = default;

    /**
     * @brief 处理OBJ格式数据的主要接口函数
     * 完成整个处理流程的核心，包括以下主要步骤：
     * 1. 使用geometry模块构建半边结构
     * 2. 验证网格有效性
     * 3. 执行几何处理操作
     * 4. 转换结果为Qt格式
     * 
     * @param vertices 输入的顶点位置数组（QVector3D格式）
     * @param indices 三角形索引数组
     * @return 处理后的顶点和索引数据对
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);

    /**
     * @brief 获取当前的半边网格（只读）
     * @return 半边网格的常量引用
     */
    const geometry::HalfEdgeMesh& getMesh() const { return mesh; }
    
    /**
     * @brief 获取当前的半边网格（可写，用于ARAP初始化）
     * @return 半边网格的引用
     */
    geometry::HalfEdgeMesh& getMesh() { return mesh; }

    /**
     * @brief 检查当前网格是否有效
     * @return 如果网格有效返回true，否则返回false
     */
    bool isValid() const { return mesh.isValid(); }

    /**
     * @brief 清空当前网格数据
     */
    void clear() { mesh.clear(); }

    // ================== ARAP交互API方法 ==================
    /**
     * @brief 开始ARAP变形会话
     * 作用：记录当前所有顶点位置为old_position（变形前的参考位置）
     *  并清空所有fixed/handle标记，准备新的交互会话
     * 调用时机：GUI进入ARAP模式时
     */
    void beginArapSession();
    
    /**
     * @brief 结束ARAP变形会话，恢复正常模式
     * 作用：清理所有fixed/handle标记
     * 调用时机：GUI退出ARAP模式时
     */
    void endArapSession();
    
    /**
     * @brief 设置单个顶点的固定状态
     * @param index 顶点索引（范围应为[0, vertexCount)）
     * @param fixed true=固定该顶点，false=取消固定
     * 作用：将这些顶点在ARAP变形时作为位置约束（锚点）
     * 调用时机：用户在GUI中点击选择固定点时
     */
    void setFixedVertex(int index, bool fixed = true);
    
    /**
     * @brief 批量设置多个顶点为固定状态
     * @param indices 顶点索引数组
  * 作用：一次性设置多个固定点，性能更高
     */
    void setFixedVertices(const std::vector<int>& indices);
    
    /**
     * @brief 设置单个顶点为handle点（拖拽控制点）
     * @param index 顶点索引
     * @param handle true=设置为handle，false=取消handle
  * 注意：同一时间只能有一个handle点，设置新handle会自动清除旧的
     */
    void setHandleVertex(int index, bool handle = true);
    
    /**
     * @brief 清除所有顶点的固定标记
     * 作用：撤销固定点选择，方便用户重新选择
     * 调用时机：用户点击"clear fixed"按钮时
     */
    void clearFixedVertices();
    
    /**
     * @brief 清除handle点标记
     */
    void clearHandleVertex();
    
    /**
     * @brief 获取当前handle点的索引
     * @return handle点索引，如果没有返回-1
 */
    int getHandleIndex() const;
    
    /**
     * @brief 应用ARAP拖拽变形
     * @param handleIndex handle点的索引
     * @param newPosition handle点的新位置（用户拖动后的3D位置）
     * @return 变形后的网格数据（新顶点位置和索引）
     * 
     * 作用：根据handle点的新位置和固定点约束，执行ARAP算法优化
     *       其他顶点的位置，实现变形（as-rigid-as-possible）效果
     * 
     * 前提条件：必须已设置固定点
     * 如果handleIndex无效，直接返回当前mesh
     * 
     * 处理流程：
     *   1. 设置handleIndex为handle点
  *   2. 更新handle点位置
     *   3. 将handle点也标记为fixed（作为位置约束）
     *   4. 调用processGeometry()执行ARAP优化
     *   5. 返回变形后mesh给GUI显示
     * 
     * 调用时机：用户拖动handle点时，每次鼠标移动都调用
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    applyArapDrag(int handleIndex, const QVector3D& newPosition);

private:
    geometry::HalfEdgeMesh mesh;  ///< 半边网格对象，存储转换后的网格数据

    /**
     * @brief 执行具体的几何处理操作
     * 这是需要自定义网格处理算法的地方
     * 例如：拉普拉斯平滑、网格简化、细分等等
     */
    void processGeometry();

    void tuttes_embedding();

    double wij_caculate(geometry::HalfEdge* he, int i);
};