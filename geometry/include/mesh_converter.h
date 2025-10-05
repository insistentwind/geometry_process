#ifndef GEOMETRY_MESH_CONVERTER_H
#define GEOMETRY_MESH_CONVERTER_H

#include <QVector3D>
#include <vector>
#include <utility>
#include <Eigen/Dense>
#include "halfedge.h"

namespace geometry {

/**
 * @brief �����ʽת��������
 * �ṩQVector3D��Eigen::Vector3d֮���ת��
 * �Լ������������߽ṹ֮���ת��
 * �������Ա�������ҪQt��������ת����hw��Ŀ����
 */
class MeshConverter {
public:
    /**
     * @brief ��QVector3D����ת��ΪEigen::Vector3d����
     * @param qVertices QVector3D��ʽ�Ķ�������
     * @return Eigen::Vector3d��ʽ�Ķ�������
     */
    static std::vector<Eigen::Vector3d> convertQtToEigen(const std::vector<QVector3D>& qVertices);

    /**
     * @brief ��Eigen::Vector3d����ת��ΪQVector3D����
     * @param eigenVerts Eigen::Vector3d��ʽ�Ķ�������
     * @return QVector3D��ʽ�Ķ�������
     */
    static std::vector<QVector3D> convertEigenToQt(const std::vector<Eigen::Vector3d>& eigenVerts);

    /**
     * @brief ����������������������ת��Ϊ��ṹ
     * @param indices �������������飨ÿ3��Ϊһ�������Σ�
     * @return ��ṹ���飬ÿ��������䶥������
     */
    static std::vector<std::vector<int>> convertIndicesToFaces(const std::vector<unsigned int>& indices);

    /**
     * @brief ��Qt��ʽ���ݹ����������
     * @param mesh Ŀ��������
     * @param qVertices QVector3D��ʽ�Ķ�������
     * @param indices ��������������
     */
    static void buildMeshFromQtData(HalfEdgeMesh& mesh,
                                    const std::vector<QVector3D>& qVertices,
                                    const std::vector<unsigned int>& indices);

    /**
     * @brief ���������ת��ΪQt��ʽ����
     * @param mesh Դ�������
     * @return ������������������pair
     */
    static std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    convertMeshToQtData(const HalfEdgeMesh& mesh);
};

} // namespace geometry

#endif // GEOMETRY_MESH_CONVERTER_H
