# ��������Ŀ�ṹ

## ��Ŀ����

�����Ŀʹ�ð�����ݽṹ��������ά����֧�ֶ����ҵ��Ŀ����ͬһ�����ο⡣

## ��Ŀ�ṹ

```
mesh_works/
������ CMakeLists.txt           # ��CMake�����ļ�
������ GEOMETRY_README.md      # ��Ŀ˵���ĵ�
������ geometry/               # �����ο�
��   ������ CMakeLists.txt      # ���ο�CMake����
��   ������ include/
��   ��   ������ halfedge.h      # ������ݽṹͷ�ļ�
��   ������ src/
��       ������ halfedge.cpp    # ������ݽṹʵ��
������ viewer/                 # �����3D�鿴����
��   ������ CMakeLists.txt
��   ������ include/
��   ������ src/
������ src/                    # ������ҵ��Ŀ
��   ������ hw1/               # ��ҵ1
��   ��   ������ CMakeLists.txt
��   ��   ������ src/
��   ��       ������ hw1.cpp
��   ������ hw2/               # ��ҵ2 (ʾ��)
��   ��   ������ CMakeLists.txt
��   ��   ������ src/
��   ��       ������ hw2.cpp
��   ������ ...                # ������ҵ��Ŀ
������ thirdparty/            # �������⣨��Eigen��
```

## ���ο�����

### ������ݽṹ (`geometry::HalfEdgeMesh`)

- **�����ռ�**: `geometry`
- **ͷ�ļ�**: `#include <halfedge.h>`
- **����**:
  - ��Ч���������˱�ʾ
  - ֧�ָ��ӵ��������
  - ���������ݽṹ��֤
  - ��ϸ���ĵ��ʹ�����

### ��Ҫ��

1. **`geometry::Vertex`** - ������
2. **`geometry::Face`** - ����
3. **`geometry::HalfEdge`** - �����
4. **`geometry::HalfEdgeMesh`** - ���������

## ��δ����µ���ҵ��Ŀ

�ο� `src/hw2/` Ŀ¼��ʵ�֣�ֻ��Ҫ��

1. �����µ���ĿĿ¼
2. ���CMakeLists.txt������ `geometry::halfedge`��
3. ���������� `#include <halfedge.h>`
4. ʹ�� `geometry::HalfEdgeMesh` ����������

## ���������

```bash
cmake -B build -G Ninja
cmake --build build
./build/bin/hw1.exe
./build/bin/hw2.exe
```