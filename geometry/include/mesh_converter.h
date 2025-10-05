#ifndef GEOMETRY_MESH_CONVERTER_H
#define GEOMETRY_MESH_CONVERTER_H

#include <QVector3D>
#include <vector>
#include <utility>
#include <Eigen/Dense>
#include "halfedge.h"

namespace geometry {

/**
 * @brief 网格格式转换工具类
 * 提供QVector3D与Eigen::Vector3d之间的转换
 * 以及网格数据与半边结构之间的转换
 * 这个类可以被所有需要Qt与半边网格转换的hw项目复用
 */
class MeshConverter {
public:
    /**
     * @brief 将QVector3D数组转换为Eigen::Vector3d数组
     * @param qVertices QVector3D格式的顶点数组
     * @return Eigen::Vector3d格式的顶点数组
     */
    static std::vector<Eigen::Vector3d> convertQtToEigen(const std::vector<QVector3D>& qVertices);

    /**
     * @brief 将Eigen::Vector3d数组转换为QVector3D数组
     * @param eigenVerts Eigen::Vector3d格式的顶点数组
     * @return QVector3D格式的顶点数组
     */
    static std::vector<QVector3D> convertEigenToQt(const std::vector<Eigen::Vector3d>& eigenVerts);

    /**
     * @brief 将连续的三角形索引数组转换为面结构
     * @param indices 三角形索引数组（每3个为一个三角形）
     * @return 面结构数组，每个面包含其顶点索引
     */
    static std::vector<std::vector<int>> convertIndicesToFaces(const std::vector<unsigned int>& indices);

    /**
     * @brief 从Qt格式数据构建半边网格
     * @param mesh 目标半边网格
     * @param qVertices QVector3D格式的顶点数组
     * @param indices 三角形索引数组
     */
    static void buildMeshFromQtData(HalfEdgeMesh& mesh,
                                    const std::vector<QVector3D>& qVertices,
                                    const std::vector<unsigned int>& indices);

    /**
     * @brief 将半边网格转换为Qt格式数据
     * @param mesh 源半边网格
     * @return 顶点数组和索引数组的pair
     */
    static std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    convertMeshToQtData(const HalfEdgeMesh& mesh);
};

} // namespace geometry

#endif // GEOMETRY_MESH_CONVERTER_H
