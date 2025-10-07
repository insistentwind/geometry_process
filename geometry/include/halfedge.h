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

class Vertex {
public:
    Eigen::Vector3d position;
    Eigen::Vector3d normal;
    Eigen::Vector2d texCoords;
    Eigen::Vector3d color;
    HalfEdge* halfEdge;
    int index;
    Vertex(const Eigen::Vector3d& pos, int idx)
        : position(pos), normal(Eigen::Vector3d::Zero()), texCoords(Eigen::Vector2d::Zero()),
          color(Eigen::Vector3d::Ones()), halfEdge(nullptr), index(idx) {}
    Vertex(const Eigen::Vector3d& pos, const Eigen::Vector3d& norm,
           const Eigen::Vector2d& tex, const Eigen::Vector3d& col, int idx)
        : position(pos), normal(norm), texCoords(tex), color(col), halfEdge(nullptr), index(idx) {}
    void computeNormal();
    void normalizeNormal() { if (normal.norm() > 0) normal.normalize(); }
    int getDegree() const;
    bool isBoundary() const;
};

class Face {
public:
    HalfEdge* halfEdge;
    int index;
    Eigen::Vector3d normal;
    Face(int idx) : halfEdge(nullptr), index(idx), normal(Eigen::Vector3d::Zero()) {}
    void computeNormal();
    double computeArea() const;
    int getVertexCount() const;
};

class HalfEdge {
public:
    Vertex* vertex;   ///< 起点
    Face* face;       ///< 左侧面
    HalfEdge* next;   ///< 下一条
    HalfEdge* prev;   ///< 上一条
    HalfEdge* pair;   ///< 对偶
    Eigen::Vector3d edgeColor; ///< 边颜色(默认白色)，可用于高亮特定边

    HalfEdge() : vertex(nullptr), face(nullptr), next(nullptr), prev(nullptr), pair(nullptr), edgeColor(Eigen::Vector3d::Ones()) {}

    double getLength() const;
    Eigen::Vector3d getDirection() const;
    bool isBoundary() const { return pair == nullptr; }
    Vertex* getEndVertex() const { return next->vertex; }
};

class HalfEdgeMesh {
public:
    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<Face>> faces;
    std::vector<std::unique_ptr<HalfEdge>> halfEdges;
    HalfEdgeMesh() = default;
    HalfEdgeMesh(const HalfEdgeMesh&) = delete;
    HalfEdgeMesh& operator=(const HalfEdgeMesh&) = delete;
    HalfEdgeMesh(HalfEdgeMesh&&) = default;
    HalfEdgeMesh& operator=(HalfEdgeMesh&&) = default;
    void buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                      const std::vector<std::vector<int>>& faceIndices);
    void buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                      const std::vector<Eigen::Vector3d>& vertexNormals,
                      const std::vector<Eigen::Vector2d>& vertexTexCoords,
                      const std::vector<std::vector<int>>& faceIndices);
    void clear();
    void computeNormals();
    size_t getVertexCount() const { return vertices.size(); }
    size_t getFaceCount() const { return faces.size(); }
    size_t getHalfEdgeCount() const { return halfEdges.size(); }
    bool isEmpty() const { return vertices.empty(); }
    bool isValid() const;
    std::pair<Eigen::Vector3d, Eigen::Vector3d> getBoundingBox() const;
    double getTotalSurfaceArea() const;
private:
    HalfEdge* getOrCreateHalfEdgePair(Vertex* v1, Vertex* v2,
                                      std::map<std::pair<int,int>, HalfEdge*>& edgeMap,
                                      HalfEdge* currentHalfEdge);
};

} // namespace geometry

#endif // GEOMETRY_HALFEDGE_H