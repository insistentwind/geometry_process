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
 * ְ��:
 *  1. Qt QVector3D <-> Eigen::Vector3d ��ת
 *  2. �������������� <-> �涥�������б� ��ת
 *  3. Qt ԭʼ(����+����)���� �� HalfEdgeMesh ֮�乹��/��д
 *
 * ˵��:
 *  - ��������ֻ�������ݰ���/��ʽת����, �������Ӽ��δ���.
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
     * @brief convertIndicesToFaces  ��ƽ������������תΪ������֯(ÿ��һ��int�б�)
     * ���� indices.size() �� 3 �ı���
     */
    static std::vector<std::vector<int>> convertIndicesToFaces(const std::vector<unsigned int>& indices);

    /**
     * @brief buildMeshFromQtData  ʹ�� Qt ���� + ���������� �����������
     * @param mesh      ��� HalfEdgeMesh (�ڲ�������ؽ�)
     * @param qVertices �����б� (λ��)
     * @param indices   �����ζ������� (ÿ3�����һ��������)
     */
    static void buildMeshFromQtData(HalfEdgeMesh& mesh,
                                    const std::vector<QVector3D>& qVertices,
                                    const std::vector<unsigned int>& indices);

    /**
     * @brief convertMeshToQtData  HalfEdgeMesh -> (QVector3D ��������, ��������������)
     * @return pair(��������, ��������)
     */
    static std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
    convertMeshToQtData(const HalfEdgeMesh& mesh);
};

} // namespace geometry

#endif // GEOMETRY_MESH_CONVERTER_H
