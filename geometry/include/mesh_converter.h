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
 *1. Qt QVector3D <-> Eigen::Vector3d 相互转换
 *2. 三角面片索引数组 <-> 面列表(每面一个int列表)互转
 *3. Qt 原始(顶点+索引)数据 与 HalfEdgeMesh之间构建/写回
 *
 *说明:
 * -只做数据搬运/格式转换, 不做额外几何计算.
 */
class MeshConverter {
public:
 /**
 * @brief convertQtToEigen QVector3D -> Eigen::Vector3d
 */
 static std::vector<Eigen::Vector3d> convertQtToEigen(const std::vector<QVector3D>& qVertices);

 /**
 * @brief convertEigenToQt Eigen::Vector3d -> QVector3D
 */
 static std::vector<QVector3D> convertEigenToQt(const std::vector<Eigen::Vector3d>& eigenVerts);

 /**
 * @brief convertIndicesToFaces 将平面三角形索引转为面结构(每面一个int列表)
 * 要求 indices.size() 为3 的倍数
 */
 static std::vector<std::vector<int>> convertIndicesToFaces(const std::vector<unsigned int>& indices);

 /**
 * @brief buildMeshFromQtData 使用 Qt 顶点 + 三角索引 构建半边数据结构
 * @param mesh 输出 HalfEdgeMesh (内部将被重建)
 * @param qVertices 顶点列表 (位置)
 * @param indices 三角面片顶点索引 (每3个组成一个三角形)
 */
 static void buildMeshFromQtData(HalfEdgeMesh& mesh,
 const std::vector<QVector3D>& qVertices,
 const std::vector<unsigned int>& indices);

 /**
 * @brief convertMeshToQtData HalfEdgeMesh -> (QVector3D 顶点数组, 三角索引数组)
 * @return pair(顶点数组, 索引数组)
 */
 static std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
 convertMeshToQtData(const HalfEdgeMesh& mesh);
};

} // namespace geometry

#endif // GEOMETRY_MESH_CONVERTER_H
