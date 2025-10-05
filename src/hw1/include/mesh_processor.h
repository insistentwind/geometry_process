#pragma once

#include <QVector3D>
#include <vector>
#include <utility>
#include <halfedge.h>

/**
 * @brief ���������࣬���ڽ�OBJ����ת��Ϊ��߽ṹ�����м��δ���
 * ��Ҫ�������ݸ�ʽת������߽ṹ���������δ���ͽ��ת��
 * ��UI��ȫ���룬רע���������㷨
 */
class MeshProcessor {
public:
    /**
     * @brief ���캯��
     */
    MeshProcessor() = default;

    /**
     * @brief ��������
     */
    ~MeshProcessor() = default;

    /**
     * @brief ����OBJ�������ݵ���Ҫ��ں���
     * �����������̵ĺ��ģ�����������Ҫ���裺
     * 1. ���ݸ�ʽת����QVector3D -> Eigen::Vector3d��
     * 2. �����������ṹ
     * 3. ��֤������Ч��
     * 4. ִ�м��δ������
     * 5. ת�������Qt��ʽ
     * 
     * @param vertices ����Ķ����������飨QVector3D��ʽ��
     * @param indices ���������������
     * @return �����Ķ�����������ݶ�
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);

    /**
     * @brief ��ȡ��ǰ�İ���������ֻ����
     * @return �������ĳ�������
     */
    const geometry::HalfEdgeMesh& getMesh() const { return mesh; }

    /**
     * @brief ��鵱ǰ�����Ƿ���Ч
     * @return ���������Ч����true�����򷵻�false
     */
    bool isValid() const { return mesh.isValid(); }

    /**
     * @brief ��յ�ǰ��������
     */
    void clear() { mesh.clear(); }

private:
    geometry::HalfEdgeMesh mesh;  ///< ���������󣬴洢ת�������������

    /**
     * @brief ִ�о���ļ��δ������
     * ����������Զ����������㷨�ĵط�
     * ���磺������˹ƽ��������򻯡�ϸ�������
     */
    void processGeometry();

    /**
     * @brief ��Qt��ʽת��ΪEigen��ʽ
     * @param vertices QVector3D��ʽ�Ķ�������
     * @return Eigen::Vector3d��ʽ�Ķ�������
     */
    std::vector<Eigen::Vector3d> convertToEigen(const std::vector<QVector3D>& vertices);

    /**
     * @brief ����������ת��Ϊ��ṹ
     * @param indices ��������������������
     * @return ��ṹ���飬ÿ��������䶥������
     */
    std::vector<std::vector<int>> convertToFaces(const std::vector<unsigned int>& indices);

    /**
     * @brief ������������ת����Qt��ʽ
     * @return �����������pair
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> convertFromMesh();
};