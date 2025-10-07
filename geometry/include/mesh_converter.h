#ifndef GEOMETRY_MESH_CONVERTER_H
#define GEOMETRY_MESH_CONVERTER_H

#include <QVector3D>
#include <vector>
#include <utility>
#include <Eigen/Dense>
#include "halfedge.h"

namespace geometry {

/**
 * @brief MeshConverter
 * 职责:
 *  1. Qt QVector3D <-> Eigen::Vector3d 互转
 *  2. 三角形索引数组 <-> 面顶点索引列表 互转
 *  3. Qt 原始(顶点+索引)数据 与 HalfEdgeMesh 之间构建/回写
 *
 * 说明:
 *  - 本工具类只做“数据搬运/格式转换”, 不做复杂几何处理.
 */
class MeshConverter {
public:
    /**
     * @brief convertQtToEigen  QVector3D -> Eigen::Vector3d
     */
    static std::vector<Eigen::Vector3d> convertQtToEigen(const std::vector<QVector3D>& qVertices);

    /**
     * @brief convertEigenToQt  Eigen::Vector3d -> QVector3D
     */
    static std::vector<QVector3D> convertEigenToQt(const std::vector<Eigen::Vector3d>& eigenVerts);

    /**
     * @brief convertIndicesToFaces  将平铺三角形索引转为按面组织(每面一个int列表)
     * 期望 indices.size() 是 3 的倍数
     */
    static std::vector<std::vector<int>> convertIndicesToFaces(const std::vector<unsigned int>& indices);

    /**
     * @brief buildMeshFromQtData  使用 Qt 顶点 + 三角形索引 构建半边网格
     * @param mesh      输出 HalfEdgeMesh (内部会清空重建)
     * @param qVertices 顶点列表 (位置)
     * @param indices   三角形顶点索引 (每3个组成一个三角形)
     */
    static void buildMeshFromQtData(HalfEdgeMesh& mesh,
                                    const std::vector<QVector3D>& qVertices,
                                    const std::vector<unsigned int>& indices);

    /**
     * @brief convertMeshToQtData  HalfEdgeMesh -> (QVector3D 顶点数组, 三角形索引数组)
     * @return pair(顶点数组, 索引数组)
     */
    static std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
    convertMeshToQtData(const HalfEdgeMesh& mesh);
};

} // namespace geometry

#endif // GEOMETRY_MESH_CONVERTER_H
