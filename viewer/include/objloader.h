#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <QVector3D>

class ObjLoader {
public:
    bool loadOBJ(const std::string& path);

    std::vector<QVector3D> vertices;
    std::vector<QVector3D> normals;
    std::vector<unsigned int> indices;
};

#endif // OBJLOADER_H