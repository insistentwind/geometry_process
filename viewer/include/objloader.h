#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <QVector3D>

/**
 * @brief 简单 OBJ 文件加载器 (仅支持 v / f 基础数据)
 * 不支持: 纹理坐标 / 法线 / 材质 / 四边及以上多边形
 */
class ObjLoader {
public:
    /**
     * @brief 加载 OBJ
     * @param path 文件路径
     * @return 成功返回 true
     */
    bool loadOBJ(const std::string& path);

    std::vector<QVector3D> vertices;   ///< 顶点位置
    std::vector<QVector3D> colors;     ///< (未使用) 可扩展
    std::vector<QVector3D> normals;    ///< (未填充) 可扩展
    std::vector<unsigned int> indices; ///< 三角形索引 (每3个为一面)
};

#endif // OBJLOADER_H