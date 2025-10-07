#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>
#include <QVector3D>

/**
 * @brief �� OBJ �ļ������� (��֧�� v / f ��������)
 * ��֧��: �������� / ���� / ���� / �ı߼����϶����
 */
class ObjLoader {
public:
    /**
     * @brief ���� OBJ
     * @param path �ļ�·��
     * @return �ɹ����� true
     */
    bool loadOBJ(const std::string& path);

    std::vector<QVector3D> vertices;   ///< ����λ��
    std::vector<QVector3D> colors;     ///< (δʹ��) ����չ
    std::vector<QVector3D> normals;    ///< (δ���) ����չ
    std::vector<unsigned int> indices; ///< ���������� (ÿ3��Ϊһ��)
};

#endif // OBJLOADER_H