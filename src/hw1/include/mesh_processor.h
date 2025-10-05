#pragma once

#include <QVector3D>
#include <vector>
#include <utility>
#include <halfedge.h>

/**
 * @brief 网格处理器类，用于将OBJ数据转换为半边结构并进行几何处理
 * 主要负责数据格式转换、半边结构构建、几何处理和结果转换
 * 与UI完全分离，专注于网格处理算法
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
     * @brief 处理OBJ网格数据的主要入口函数
     * 整个处理流程的核心，包含以下主要步骤：
     * 1. 数据格式转换（QVector3D -> Eigen::Vector3d）
     * 2. 构建半边网格结构
     * 3. 验证网格有效性
     * 4. 执行几何处理操作
     * 5. 转换结果回Qt格式
     * 
     * @param vertices 输入的顶点坐标数组（QVector3D格式）
     * @param indices 输入的面索引数组
     * @return 处理后的顶点和索引数据对
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);

    /**
     * @brief 获取当前的半边网格对象（只读）
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
     * 这是你可以自定义网格处理算法的地方
     * 例如：拉普拉斯平滑、网格简化、细分曲面等
     */
    void processGeometry();

    /**
     * @brief 将Qt格式转换为Eigen格式
     * @param vertices QVector3D格式的顶点数组
     * @return Eigen::Vector3d格式的顶点数组
     */
    std::vector<Eigen::Vector3d> convertToEigen(const std::vector<QVector3D>& vertices);

    /**
     * @brief 将索引数组转换为面结构
     * @param indices 连续的三角形索引数组
     * @return 面结构数组，每个面包含其顶点索引
     */
    std::vector<std::vector<int>> convertToFaces(const std::vector<unsigned int>& indices);

    /**
     * @brief 将处理后的网格转换回Qt格式
     * @return 顶点和索引的pair
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> convertFromMesh();
};