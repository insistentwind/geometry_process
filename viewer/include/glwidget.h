#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector2D>
#include "objloader.h"
#include <QOpenGLShaderProgram>
#include <QString>
#include <vector>
#include <set>
#include <functional>

/**
 * @brief OpenGL��Ⱦ�����֧࣬��������ʾ��ARAP����
 * 
 * ����ARAP���ܣ�
 * - ����/�˳�ARAPģʽ
 * - �����ʰȡ���㣨����fixed�㣩
 * - �����ק���㣨��Ϊhandle���б��Σ�
 */
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    enum class ColorDisplayMode { PointsOnly, Faces };
    
    // ==================== ARAPѡ��ģʽ ====================
    /**
     * @brief ARAPѡ��ģʽö��
     * SelectFixed: ѡ��̶���ģʽ���ɶ�ѡ��
     * SelectHandle: ѡ���϶���ģʽ����ѡ��
     * None: ��ѡ�񣬿����϶���ͼ
     */
    enum class ArapSelectionMode {
     None,          // ����ģʽ������ת��ͼ��
        SelectFixed,   // ѡ��Fixed��ģʽ
        SelectHandle   // ѡ��Handle��ģʽ
    };

    explicit GLWidget(QWidget *parent = nullptr);
    ~GLWidget() override;

    void updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices);
    void updateMeshWithColors(const std::vector<QVector3D>& vertices,
     const std::vector<unsigned int>& indices,
       const std::vector<QVector3D>& colorsIn);
    void updateMSTEdges(const std::vector<std::pair<int,int>>& edges);
    void clearMSTEdges();

    bool loadObject(const QString& fileName);
    const std::vector<QVector3D>& getVertices() const;
    const std::vector<unsigned int>& getIndices() const;

    void setWireframe(bool enabled) { wireframe = enabled; update(); }
    void setShowColoredPoints(bool enabled) { showColoredPoints = enabled; update(); }
    void setPointSize(float psz) { pointSize = psz; update(); }
    void setColorDisplayMode(ColorDisplayMode m) { colorMode = m; update(); }
    void cycleColorDisplayMode();
    ColorDisplayMode getColorDisplayMode() const { return colorMode; }
    void toggleFilledFaces() { showFilledFaces = !showFilledFaces; update(); }
    bool isShowingFilledFaces() const { return showFilledFaces; }

    // ==================== ARAP����API ====================
    /**
     * @brief �л�ARAPģʽ����
     * ���ã�����ARAPģʽ��������ѡ��fixed���㣬��קʵ�ֱ���
     */
    void toggleArapMode();
    
    /**
     * @brief ��ѯ��ǰ�Ƿ���ARAPģʽ
     */
    bool isArapActive() const { return arapActive; }
    
    /**
     * @brief ���������ѡ���fixed����
     * ���ã���չ̶���Լ��
     */
    void clearArapFixedVertices();
    
    /**
     * @brief ����ARAPѡ��ģʽ
     * @param mode ѡ��ģʽ��None/SelectFixed/SelectHandle��
     */
    void setArapSelectionMode(ArapSelectionMode mode);
    
    /**
     * @brief ��ȡ��ǰARAPѡ��ģʽ
     */
    ArapSelectionMode getArapSelectionMode() const { return arapSelectionMode; }

    // ==================== ARAP�ص���������hw6.cpp�����ã� ====================
    /**
     * @brief ARAP�Ự��ʼ�ص�
     * �ⲿ���ô˺�������ʼ��ARAP�㷨������old_position�ȣ�
     * ����ʱ��������ARAPģʽʱ
     */
    std::function<void()> arapBeginCallback;
    
    /**
* @brief ���ù̶�����ص�
     * @param int ��������
     * @param bool �Ƿ�̶�
     * �ⲿ���ô˺����������Щ������fixed��
     * ����ʱ�����û��������ʱ
     */
    std::function<void(int, bool)> arapSetFixedCallback;
    
    /**
     * @brief �������fixed����ص�
     * �ⲿ���ô˺�����������й̶�����
     * ����ʱ�����û����"clear fixed"��ťʱ
     */
    std::function<void()> arapClearFixedCallback;
    
    /**
     * @brief ARAP��ק���λص�
     * @param int handle��������
     * @param QVector3D handle����λ��
     * @return ���κ���������ݣ������������
     * �ⲿ���ô˺�����ִ��ARAP�㷨��������mesh
     * ����ʱ�����û���ק����ʱ��ʵʱ����
     */
    std::function<std::pair<std::vector<QVector3D>, std::vector<unsigned int>>(int, const QVector3D&)> arapDragCallback;

protected:
    void initializeGL() override;
  void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
  void calculateModelBounds();
    void resetView();
    
 // ==================== ARAP�������� ====================
    /**
     * @brief ��Ļ����ʰȡ����Ķ���
     * @param pos ��Ļ���꣨���λ�ã�
     * @return ����������-1��ʾδʰȡ��
   * 
     * ʵ��ԭ��
     * 1. �����ж���ͶӰ����Ļ�ռ�
     * 2. �������λ����ÿ��������Ļλ�õľ���
     * 3. ���ؾ������������ֵ�ڵĶ���
     */
    int pickVertex(const QPoint& pos) const;
    
    /**
     * @brief ��Ļ����ת��Ϊ����3D����
     * @param pos ��Ļ����
     * @param depth ���ֵ����������ľ��룩
   * @return ��������
     * 
     * ��;���������ק��2Dλ��ת��Ϊ3D�ռ���handle�����λ��
     */
    QVector3D screenToWorld(const QPoint& pos, float depth) const;
    
    /**
  * @brief ִ��ARAP��ק����
  * @param pos ��ǰ�����Ļλ��
     * 
     * �������̣�
     * 1. �����λ��ת��Ϊ3D���꣨����ԭ��ȣ�
     * 2. ����arapDragCallbackִ��ARAP�㷨
     * 3. ����mesh��ʾ
     */
    void performArapDrag(const QPoint& pos);
    
    /**
     * @brief ��ʼARAP�Ự�������Ҫ��
   * ����arapBeginCallback��ʼ��
     */
    void beginArapIfNeeded();

    // ԭ�г�Ա����
    ObjLoader objLoader;
    QOpenGLShaderProgram *program {nullptr};
    QOpenGLBuffer vertexBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer indexBuf {QOpenGLBuffer::IndexBuffer};
    QOpenGLBuffer colorBuf {QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer mstLineBuf {QOpenGLBuffer::VertexBuffer};

    QPoint lastPos;
    float distance;
    float rotationX;
    float rotationY;
QVector2D panOffset;

QVector3D modelCenter;
  float modelRadius;

    std::vector<QVector3D> vertices;
    std::vector<unsigned int> indices;
    std::vector<QVector3D> colors;
    std::vector<QVector3D> mstLineVertices;

    QMatrix4x4 projection;

    bool wireframe = true;
    bool perVertexColor = false;
    bool showColoredPoints = true;
    float pointSize = 3.0f;  // ��Ϊ3.0fʹ���С

    ColorDisplayMode colorMode { ColorDisplayMode::PointsOnly };
    bool showFilledFaces = false;
    
    // ==================== ARAP״̬���� ====================
    bool arapActive = false;     // �Ƿ���ARAPģʽ
    ArapSelectionMode arapSelectionMode = ArapSelectionMode::None; // ARAPѡ��ģʽ
    int arapHandleVertex = -1; // ��ǰ��ק��handle����������-1��ʾδѡ��
    std::set<int> fixedVertices; // ���б��Ϊfixed�Ķ�����������
    bool leftButtonDraggingHandle = false;// �Ƿ�������קhandle
    float handleDepth = 0.0f;     // handle����ĳ�ʼ��ȣ�������Ļת�������꣩
};

#endif // GLWIDGET_H