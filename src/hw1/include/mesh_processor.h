#pragma once

#include <QVector3D>
#include <vector>
#include <utility>
#include <halfedge.h>

/**
 * @brief �������࣬���ڽ�OBJ����ת��Ϊ��߽ṹ�����м��δ���
 * �������geometryģ���ת�����ܺ�ʵ�־���ļ��δ����㷨
 * ��UI��ȫ���רע���������㷨
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
     * @brief ����OBJ��ʽ���ݵ���Ҫ�ӿں���
     * ��������������̵ĺ��ģ�����������Ҫ���裺
     * 1. ʹ��geometryģ�鹹����߽ṹ
     * 2. ��֤������Ч��
     * 3. ִ�м��δ������
     * 4. ת�����ΪQt��ʽ
     * 
     * @param vertices ����Ķ���λ�����飨QVector3D��ʽ��
     * @param indices ��������������
     * @return �����Ķ�����������ݶ�
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices);

    /**
     * @brief ��ȡ��ǰ�İ������ֻ����
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
     * ������Ҫ�Զ����������㷨�ĵط�
     * ���磺������˹ƽ��������򻯡�ϸ�ֵȵ�
     */
    void processGeometry(std::vector<int>& indexes);

    std::vector<double> dijkstra(int start_index, geometry::HalfEdgeMesh& mesh);

    int find_min_distance_node(
        const std::vector<double>& dist,
        const std::vector<bool>& visited);
    
};