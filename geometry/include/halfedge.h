#ifndef GEOMETRY_HALFEDGE_H
#define GEOMETRY_HALFEDGE_H

#include <vector>
#include <map>
#include <memory>
#include <Eigen/Dense>

namespace geometry {

class Vertex;
class Face;
class HalfEdge;

/**
 * @brief ��߽ṹ�еĶ�����
 * ���������Ķ������ԣ�λ�á������������������
 */
class Vertex {
public:
    Eigen::Vector3d position;      ///< ����λ�� (x, y, z)
    Eigen::Vector3d normal;        ///< ���㷨���� (nx, ny, nz)
    Eigen::Vector2d texCoords;     ///< �������� (u, v)
    Eigen::Vector3d color;         ///< ������ɫ (r, g, b) ��Χ[0,1]
    HalfEdge* halfEdge;           ///< �Ӹö������������һ�����
    int index;                    ///< ��������

    /**
     * @brief ���캯�� - ��λ��
     * @param pos ����λ��
     * @param idx ��������
     */
    Vertex(const Eigen::Vector3d& pos, int idx) 
        : position(pos), 
          normal(Eigen::Vector3d::Zero()),       // Ĭ�Ϸ�����Ϊ������
          texCoords(Eigen::Vector2d::Zero()),    // Ĭ����������Ϊ(0,0)
          color(Eigen::Vector3d::Ones()),        // Ĭ����ɫΪ��ɫ(1,1,1)
          halfEdge(nullptr), 
          index(idx) {}

    /**
     * @brief �������캯��
     * @param pos ����λ��
     * @param norm ������
     * @param tex ��������
     * @param col ������ɫ
     * @param idx ��������
     */
    Vertex(const Eigen::Vector3d& pos, 
           const Eigen::Vector3d& norm,
           const Eigen::Vector2d& tex,
           const Eigen::Vector3d& col,
           int idx) 
        : position(pos), 
          normal(norm), 
          texCoords(tex), 
          color(col),
          halfEdge(nullptr), 
          index(idx) {}

    /**
     * @brief ���㲢���÷������������������ƽ����
     * ע�⣺��Ҫ�ڰ�߽ṹ���������
     */
    void computeNormal();

    /**
     * @brief ��׼��������
     */
    void normalizeNormal() {
        if (normal.norm() > 0) {
            normal.normalize();
        }
    }

    /**
     * @brief ��ȡ������������ڱߵ�������
     * @return �������
     */
    int getDegree() const;

    /**
     * @brief ����Ƿ�Ϊ�߽綥��
     * @return ����Ǳ߽綥�㷵��true
     */
    bool isBoundary() const;
};

/**
 * @brief ��߽ṹ�е�����
 */
class Face {
public:
    HalfEdge* halfEdge;          ///< �������һ�����
    int index;                   ///< ������
    Eigen::Vector3d normal;      ///< �淨����

    /**
     * @brief ���캯��
     * @param idx ������
     */
    Face(int idx) : halfEdge(nullptr), index(idx), normal(Eigen::Vector3d::Zero()) {}

    /**
     * @brief �����淨����
     */
    void computeNormal();

    /**
     * @brief �������
     * @return ���
     */
    double computeArea() const;

    /**
     * @brief ��ȡ��Ķ�����
     * @return ��������
     */
    int getVertexCount() const;
};

/**
 * @brief �����
 */
class HalfEdge {
public:
    Vertex* vertex;              ///< ��ߵ����
    Face* face;                  ///< ���������
    HalfEdge* next;             ///< ��һ�����
    HalfEdge* prev;             ///< ��һ�����
    HalfEdge* pair;             ///< ��ż���

    /**
     * @brief Ĭ�Ϲ��캯��
     */
    HalfEdge() : vertex(nullptr), face(nullptr), 
                 next(nullptr), prev(nullptr), pair(nullptr) {}

    /**
     * @brief ��ȡ��ߵĳ���
     * @return �߳�
     */
    double getLength() const;

    /**
     * @brief ��ȡ��ߵķ�������
     * @return ����㵽�յ������
     */
    Eigen::Vector3d getDirection() const;

    /**
     * @brief ����Ƿ�Ϊ�߽��
     * @return ����Ǳ߽�߷���true
     */
    bool isBoundary() const {
        return pair == nullptr;
    }

    /**
     * @brief ��ȡ��ߵ��յ�
     * @return �յ㶥��ָ��
     */
    Vertex* getEndVertex() const {
        return next->vertex;
    }
};

/**
 * @brief ��߽ṹ��������
 * 
 * �����ʵ���˰�����ݽṹ�����ڱ�ʾ����������
 * ������ݽṹ�ܹ���Ч��֧����������˲�����
 */
class HalfEdgeMesh {
public:
    std::vector<std::unique_ptr<Vertex>> vertices;   ///< ���ж���
    std::vector<std::unique_ptr<Face>> faces;        ///< ������
    std::vector<std::unique_ptr<HalfEdge>> halfEdges; ///< ���а��

    /**
     * @brief Ĭ�Ϲ��캯��
     */
    HalfEdgeMesh() = default;

    /**
     * @brief �������캯����ɾ����
     */
    HalfEdgeMesh(const HalfEdgeMesh&) = delete;

    /**
     * @brief ��ֵ�������ɾ����
     */
    HalfEdgeMesh& operator=(const HalfEdgeMesh&) = delete;

    /**
     * @brief �ƶ����캯��
     */
    HalfEdgeMesh(HalfEdgeMesh&&) = default;

    /**
     * @brief �ƶ���ֵ�����
     */
    HalfEdgeMesh& operator=(HalfEdgeMesh&&) = default;

    /**
     * @brief ��OBJ���ݹ�����߽ṹ
     * @param vertexPositions ����λ������
     * @param faceIndices ��Ķ�����������
     */
    void buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                     const std::vector<std::vector<int>>& faceIndices);

    /**
     * @brief �������������ݹ�����߽ṹ
     * @param vertexPositions ����λ������
     * @param vertexNormals ���㷨��������
     * @param vertexTexCoords ����������������
     * @param faceIndices ��Ķ�����������
     */
    void buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                     const std::vector<Eigen::Vector3d>& vertexNormals,
                     const std::vector<Eigen::Vector2d>& vertexTexCoords,
                     const std::vector<std::vector<int>>& faceIndices);

    /**
     * @brief ������������
     */
    void clear();

    /**
     * @brief �������ж������ķ�����
     */
    void computeNormals();

    /**
     * @brief ��ȡ��������
     * @return ��������
     */
    size_t getVertexCount() const { return vertices.size(); }

    /**
     * @brief ��ȡ������
     * @return ������
     */
    size_t getFaceCount() const { return faces.size(); }

    /**
     * @brief ��ȡ�������
     * @return �������
     */
    size_t getHalfEdgeCount() const { return halfEdges.size(); }

    /**
     * @brief ��������Ƿ�Ϊ��
     * @return �������Ϊ�շ���true
     */
    bool isEmpty() const { return vertices.empty(); }

    /**
     * @brief ��֤��߽ṹ��������
     * @return ����ṹ��Ч����true
     */
    bool isValid() const;

    /**
     * @brief ��������İ�Χ��
     * @return ��Χ�е���С����������
     */
    std::pair<Eigen::Vector3d, Eigen::Vector3d> getBoundingBox() const;

    /**
     * @brief ��ȡ������ܱ����
     * @return �ܱ����
     */
    double getTotalSurfaceArea() const;

private:
    /**
     * @brief ���һ򴴽���߶�
     * @param v1 ��ʼ����
     * @param v2 ��������
     * @param edgeMap ��ӳ���
     * @param currentHalfEdge ��ǰ���
     * @return ��ż���ָ�룬����������򷵻�nullptr
     */
    HalfEdge* getOrCreateHalfEdgePair(Vertex* v1, Vertex* v2,
                                     std::map<std::pair<int, int>, HalfEdge*>& edgeMap,
                                     HalfEdge* currentHalfEdge);
};

} // namespace geometry

#endif // GEOMETRY_HALFEDGE_H