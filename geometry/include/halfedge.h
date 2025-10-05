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
 * @brief 半边结构中的顶点类
 * 包含完整的顶点属性：位置、法向量、纹理坐标等
 */
class Vertex {
public:
    Eigen::Vector3d position;      ///< 顶点位置 (x, y, z)
    Eigen::Vector3d normal;        ///< 顶点法向量 (nx, ny, nz)
    Eigen::Vector2d texCoords;     ///< 纹理坐标 (u, v)
    Eigen::Vector3d color;         ///< 顶点颜色 (r, g, b) 范围[0,1]
    HalfEdge* halfEdge;           ///< 从该顶点出发的任意一条半边
    int index;                    ///< 顶点索引

    /**
     * @brief 构造函数 - 仅位置
     * @param pos 顶点位置
     * @param idx 顶点索引
     */
    Vertex(const Eigen::Vector3d& pos, int idx) 
        : position(pos), 
          normal(Eigen::Vector3d::Zero()),       // 默认法向量为零向量
          texCoords(Eigen::Vector2d::Zero()),    // 默认纹理坐标为(0,0)
          color(Eigen::Vector3d::Ones()),        // 默认颜色为白色(1,1,1)
          halfEdge(nullptr), 
          index(idx) {}

    /**
     * @brief 完整构造函数
     * @param pos 顶点位置
     * @param norm 法向量
     * @param tex 纹理坐标
     * @param col 顶点颜色
     * @param idx 顶点索引
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
     * @brief 计算并设置法向量（基于相邻面的平均）
     * 注意：需要在半边结构建立后调用
     */
    void computeNormal();

    /**
     * @brief 标准化法向量
     */
    void normalizeNormal() {
        if (normal.norm() > 0) {
            normal.normalize();
        }
    }

    /**
     * @brief 获取顶点度数（相邻边的数量）
     * @return 顶点度数
     */
    int getDegree() const;

    /**
     * @brief 检查是否为边界顶点
     * @return 如果是边界顶点返回true
     */
    bool isBoundary() const;
};

/**
 * @brief 半边结构中的面类
 */
class Face {
public:
    HalfEdge* halfEdge;          ///< 面的任意一条半边
    int index;                   ///< 面索引
    Eigen::Vector3d normal;      ///< 面法向量

    /**
     * @brief 构造函数
     * @param idx 面索引
     */
    Face(int idx) : halfEdge(nullptr), index(idx), normal(Eigen::Vector3d::Zero()) {}

    /**
     * @brief 计算面法向量
     */
    void computeNormal();

    /**
     * @brief 计算面积
     * @return 面积
     */
    double computeArea() const;

    /**
     * @brief 获取面的顶点数
     * @return 顶点数量
     */
    int getVertexCount() const;
};

/**
 * @brief 半边类
 */
class HalfEdge {
public:
    Vertex* vertex;              ///< 半边的起点
    Face* face;                  ///< 半边左侧的面
    HalfEdge* next;             ///< 下一条半边
    HalfEdge* prev;             ///< 上一条半边
    HalfEdge* pair;             ///< 对偶半边

    /**
     * @brief 默认构造函数
     */
    HalfEdge() : vertex(nullptr), face(nullptr), 
                 next(nullptr), prev(nullptr), pair(nullptr) {}

    /**
     * @brief 获取半边的长度
     * @return 边长
     */
    double getLength() const;

    /**
     * @brief 获取半边的方向向量
     * @return 从起点到终点的向量
     */
    Eigen::Vector3d getDirection() const;

    /**
     * @brief 检查是否为边界边
     * @return 如果是边界边返回true
     */
    bool isBoundary() const {
        return pair == nullptr;
    }

    /**
     * @brief 获取半边的终点
     * @return 终点顶点指针
     */
    Vertex* getEndVertex() const {
        return next->vertex;
    }
};

/**
 * @brief 半边结构的网格类
 * 
 * 这个类实现了半边数据结构，用于表示多面体网格。
 * 半边数据结构能够高效地支持网格的拓扑操作。
 */
class HalfEdgeMesh {
public:
    std::vector<std::unique_ptr<Vertex>> vertices;   ///< 所有顶点
    std::vector<std::unique_ptr<Face>> faces;        ///< 所有面
    std::vector<std::unique_ptr<HalfEdge>> halfEdges; ///< 所有半边

    /**
     * @brief 默认构造函数
     */
    HalfEdgeMesh() = default;

    /**
     * @brief 拷贝构造函数（删除）
     */
    HalfEdgeMesh(const HalfEdgeMesh&) = delete;

    /**
     * @brief 赋值运算符（删除）
     */
    HalfEdgeMesh& operator=(const HalfEdgeMesh&) = delete;

    /**
     * @brief 移动构造函数
     */
    HalfEdgeMesh(HalfEdgeMesh&&) = default;

    /**
     * @brief 移动赋值运算符
     */
    HalfEdgeMesh& operator=(HalfEdgeMesh&&) = default;

    /**
     * @brief 从OBJ数据构建半边结构
     * @param vertexPositions 顶点位置数组
     * @param faceIndices 面的顶点索引数组
     */
    void buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                     const std::vector<std::vector<int>>& faceIndices);

    /**
     * @brief 从完整顶点数据构建半边结构
     * @param vertexPositions 顶点位置数组
     * @param vertexNormals 顶点法向量数组
     * @param vertexTexCoords 顶点纹理坐标数组
     * @param faceIndices 面的顶点索引数组
     */
    void buildFromOBJ(const std::vector<Eigen::Vector3d>& vertexPositions,
                     const std::vector<Eigen::Vector3d>& vertexNormals,
                     const std::vector<Eigen::Vector2d>& vertexTexCoords,
                     const std::vector<std::vector<int>>& faceIndices);

    /**
     * @brief 清理所有数据
     */
    void clear();

    /**
     * @brief 计算所有顶点和面的法向量
     */
    void computeNormals();

    /**
     * @brief 获取顶点数量
     * @return 顶点数量
     */
    size_t getVertexCount() const { return vertices.size(); }

    /**
     * @brief 获取面数量
     * @return 面数量
     */
    size_t getFaceCount() const { return faces.size(); }

    /**
     * @brief 获取半边数量
     * @return 半边数量
     */
    size_t getHalfEdgeCount() const { return halfEdges.size(); }

    /**
     * @brief 检查网格是否为空
     * @return 如果网格为空返回true
     */
    bool isEmpty() const { return vertices.empty(); }

    /**
     * @brief 验证半边结构的完整性
     * @return 如果结构有效返回true
     */
    bool isValid() const;

    /**
     * @brief 计算网格的包围盒
     * @return 包围盒的最小和最大坐标对
     */
    std::pair<Eigen::Vector3d, Eigen::Vector3d> getBoundingBox() const;

    /**
     * @brief 获取网格的总表面积
     * @return 总表面积
     */
    double getTotalSurfaceArea() const;

private:
    /**
     * @brief 查找或创建半边对
     * @param v1 起始顶点
     * @param v2 结束顶点
     * @param edgeMap 边映射表
     * @param currentHalfEdge 当前半边
     * @return 对偶半边指针，如果不存在则返回nullptr
     */
    HalfEdge* getOrCreateHalfEdgePair(Vertex* v1, Vertex* v2,
                                     std::map<std::pair<int, int>, HalfEdge*>& edgeMap,
                                     HalfEdge* currentHalfEdge);
};

} // namespace geometry

#endif // GEOMETRY_HALFEDGE_H