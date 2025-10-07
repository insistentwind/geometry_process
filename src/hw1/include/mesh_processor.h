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
     * @brief 检查当前网格是否有效
     * @return 如果网格有效返回true，否则返回false
     */
    bool isValid() const { return mesh.isValid(); }

    /**
     * @brief 清空当前网格数据
     */
    void clear() { mesh.clear(); }

private:
    geometry::HalfEdgeMesh mesh;  ///< 半边网格对象，存储转换后的网格数据

    /**
     * @brief 执行具体的几何处理操作
     * 这是需要自定义网格处理算法的地方
     * 例如：拉普拉斯平滑、网格简化、细分等等
     */
    void processGeometry(std::vector<int>& indexes);

    std::vector<double> dijkstra(int start_index, geometry::HalfEdgeMesh& mesh);

    int find_min_distance_node(
        const std::vector<double>& dist,
        const std::vector<bool>& visited);
    
};