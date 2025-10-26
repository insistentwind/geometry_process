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
     * @brief ��ȡ��ǰ�İ�����񣨿�д������ARAP��ʼ����
     * @return ������������
     */
    geometry::HalfEdgeMesh& getMesh() { return mesh; }

    /**
     * @brief ��鵱ǰ�����Ƿ���Ч
     * @return ���������Ч����true�����򷵻�false
     */
    bool isValid() const { return mesh.isValid(); }

    /**
     * @brief ��յ�ǰ��������
     */
    void clear() { mesh.clear(); }

    // ================== ARAP����API���� ==================
    /**
     * @brief ��ʼARAP���λỰ
     * ���ã���¼��ǰ���ж���λ��Ϊold_position������ǰ�Ĳο�λ�ã�
     *  ���������fixed/handle��ǣ�׼���µĽ����Ự
     * ����ʱ����GUI����ARAPģʽʱ
     */
    void beginArapSession();
    
    /**
     * @brief ����ARAP���λỰ���ָ�����ģʽ
     * ���ã���������fixed/handle���
     * ����ʱ����GUI�˳�ARAPģʽʱ
     */
    void endArapSession();
    
    /**
     * @brief ���õ�������Ĺ̶�״̬
     * @param index ������������ΧӦΪ[0, vertexCount)��
     * @param fixed true=�̶��ö��㣬false=ȡ���̶�
     * ���ã�����Щ������ARAP����ʱ��Ϊλ��Լ����ê�㣩
     * ����ʱ�����û���GUI�е��ѡ��̶���ʱ
     */
    void setFixedVertex(int index, bool fixed = true);
    
    /**
     * @brief �������ö������Ϊ�̶�״̬
     * @param indices ������������
  * ���ã�һ�������ö���̶��㣬���ܸ���
     */
    void setFixedVertices(const std::vector<int>& indices);
    
    /**
     * @brief ���õ�������Ϊhandle�㣨��ק���Ƶ㣩
     * @param index ��������
     * @param handle true=����Ϊhandle��false=ȡ��handle
  * ע�⣺ͬһʱ��ֻ����һ��handle�㣬������handle���Զ�����ɵ�
     */
    void setHandleVertex(int index, bool handle = true);
    
    /**
     * @brief ������ж���Ĺ̶����
     * ���ã������̶���ѡ�񣬷����û�����ѡ��
     * ����ʱ�����û����"clear fixed"��ťʱ
     */
    void clearFixedVertices();
    
    /**
     * @brief ���handle����
     */
    void clearHandleVertex();
    
    /**
     * @brief ��ȡ��ǰhandle�������
     * @return handle�����������û�з���-1
 */
    int getHandleIndex() const;
    
    /**
     * @brief Ӧ��ARAP��ק����
     * @param handleIndex handle�������
     * @param newPosition handle�����λ�ã��û��϶����3Dλ�ã�
     * @return ���κ���������ݣ��¶���λ�ú�������
     * 
     * ���ã�����handle�����λ�ú͹̶���Լ����ִ��ARAP�㷨�Ż�
     *       ���������λ�ã�ʵ�ֱ��Σ�as-rigid-as-possible��Ч��
     * 
     * ǰ�����������������ù̶���
     * ���handleIndex��Ч��ֱ�ӷ��ص�ǰmesh
     * 
     * �������̣�
     *   1. ����handleIndexΪhandle��
  *   2. ����handle��λ��
     *   3. ��handle��Ҳ���Ϊfixed����Ϊλ��Լ����
     *   4. ����processGeometry()ִ��ARAP�Ż�
     *   5. ���ر��κ�mesh��GUI��ʾ
     * 
     * ����ʱ�����û��϶�handle��ʱ��ÿ������ƶ�������
     */
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    applyArapDrag(int handleIndex, const QVector3D& newPosition);

private:
    geometry::HalfEdgeMesh mesh;  ///< ���������󣬴洢ת�������������

    /**
     * @brief ִ�о���ļ��δ������
     * ������Ҫ�Զ����������㷨�ĵط�
     * ���磺������˹ƽ��������򻯡�ϸ�ֵȵ�
     */
    void processGeometry();

    double wij_caculate(geometry::HalfEdge* he, int i);
};