#include "glwidget.h"
#include <QMouseEvent>
#include <QOpenGLShader>
#include <QDebug>
#include <QtCore/QResource>
#include <QFile>
#include <cmath>
#include <limits>

GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      distance(4.0f),
      rotationX(0.0f),
      rotationY(0.0f),
      modelOffsetY(0.0f)
{
    Q_INIT_RESOURCE(resources);
    setFocusPolicy(Qt::StrongFocus);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    vertexBuf.destroy();
    indexBuf.destroy();
    colorBuf.destroy();
    delete program;
    doneCurrent();
}

void GLWidget::updateMesh(const std::vector<QVector3D>& vertices, const std::vector<unsigned int>& indices) {
    this->vertices = vertices;
    this->indices = indices;
    // Î±²Ê£¬±ÜÃâÈ«°×
    colors.resize(vertices.size());
    for(size_t i=0;i<vertices.size();++i){
        float r = fmodf((float)i * 0.37f, 1.0f);
        float g = fmodf((float)i * 0.13f, 1.0f);
        float b = fmodf((float)i * 0.73f, 1.0f);
        colors[i] = QVector3D(r,g,b);
    }

    if (!isValid()) return;

    vertexBuf.bind();
    vertexBuf.allocate(this->vertices.data(), this->vertices.size() * sizeof(QVector3D));
    indexBuf.bind();
    indexBuf.allocate(this->indices.data(), this->indices.size() * sizeof(unsigned int));
    colorBuf.bind();
    colorBuf.allocate(this->colors.data(), this->colors.size() * sizeof(QVector3D));

    calculateModelBounds();
    update();
}

const std::vector<QVector3D>& GLWidget::getVertices() const { return vertices; }
const std::vector<unsigned int>& GLWidget::getIndices() const { return indices; }

static const char* fallbackVert = R"GLSL(
#version 330 core
layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_color;
uniform mat4 u_mvp;
out vec3 vColor;void main(){vColor=a_color;gl_Position=u_mvp*vec4(a_position,1.0);} )GLSL";
static const char* fallbackFrag = R"GLSL(
#version 330 core
in vec3 vColor;out vec4 FragColor;void main(){FragColor=vec4(vColor,1.0);} )GLSL";

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.12f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    vertexBuf.create(); vertexBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    indexBuf.create();  indexBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    colorBuf.create();  colorBuf.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    program = new QOpenGLShaderProgram(this);
    bool okV=false, okF=false;
    if(QFile(":/shaders/basic.vert").exists())
        okV = program->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/basic.vert");
    if(QFile(":/shaders/basic.frag").exists())
        okF = program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/basic.frag");
    if(!okV){ qWarning()<<"Use fallback vertex shader"; program->addShaderFromSourceCode(QOpenGLShader::Vertex, fallbackVert);}    
    if(!okF){ qWarning()<<"Use fallback fragment shader"; program->addShaderFromSourceCode(QOpenGLShader::Fragment, fallbackFrag);}    
    program->bindAttributeLocation("a_position",0);
    program->bindAttributeLocation("a_color",1);
    if(!program->link()) {
        qWarning()<<"Shader link failed final:"<<program->log();
    } else {
        qDebug()<<"Shader link success";
    }
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(vertices.empty() || !program || !program->isLinked()) return;

    // ¼ÆËã MVP
    float aspect = width() > 0 ? (float)width()/(float)height() : 1.0f;
    QMatrix4x4 proj; proj.perspective(45.0f, aspect, 0.01f, 100.0f);
    QMatrix4x4 view; view.translate(0.0f, 0.0f, -distance);
    QMatrix4x4 model; model.translate(0.0f, modelOffsetY, 0.0f); model.rotate(rotationX,1,0,0); model.rotate(rotationY,0,1,0);
    QMatrix4x4 mvp = proj * view * model;

    program->bind();
    program->setUniformValue("u_mvp", mvp);

    vertexBuf.bind();
    program->enableAttributeArray(0);
    program->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));

    colorBuf.bind();
    program->enableAttributeArray(1);
    program->setAttributeBuffer(1, GL_FLOAT, 0, 3, sizeof(QVector3D));

    indexBuf.bind();
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    program->disableAttributeArray(0);
    program->disableAttributeArray(1);
    program->release();
}

void GLWidget::calculateModelBounds() {
    if (vertices.empty()) return;
    float minX=1e9f,maxX=-1e9f,minY=1e9f,maxY=-1e9f,minZ=1e9f,maxZ=-1e9f;
    for(const auto& v: vertices){
        minX = std::min(minX, v.x()); maxX = std::max(maxX, v.x());
        minY = std::min(minY, v.y()); maxY = std::max(maxY, v.y());
        minZ = std::min(minZ, v.z()); maxZ = std::max(maxZ, v.z());
    }
    float sizeX=maxX-minX,sizeY=maxY-minY,sizeZ=maxZ-minZ; float maxSize = std::max({sizeX,sizeY,sizeZ});
    if(maxSize < 1.0f && maxSize > 0.0f){
        float s = 2.0f / maxSize;
        for(auto &v: vertices){ const_cast<QVector3D&>(v) = v * s; }
        maxSize *= s; minY *= s;
    }
    distance = std::max(2.0f, std::min(maxSize * 2.0f, 50.0f));
    modelOffsetY = -minY - 0.2f;
}

bool GLWidget::loadObject(const QString& fileName) {
    if (objLoader.loadOBJ(fileName.toStdString())) {
        updateMesh(objLoader.vertices, objLoader.indices);
        return true;
    }
    return false;
}

void GLWidget::resizeGL(int width, int height) {
    projection.setToIdentity();
    if(height>0) projection.perspective(45.0f, float(width)/float(height), 0.01f, 100.0f);
}

void GLWidget::mousePressEvent(QMouseEvent *e){ lastPos = e->pos(); }
void GLWidget::mouseMoveEvent(QMouseEvent *e){
    int dx = e->x()-lastPos.x(); int dy = e->y()-lastPos.y();
    if(e->buttons() & Qt::LeftButton){ rotationY += dx * 0.5f; rotationX += dy * 0.5f; update(); }
    lastPos = e->pos();
}
void GLWidget::wheelEvent(QWheelEvent *e){ float delta = e->angleDelta().y()/120.f; distance -= delta * 0.6f; distance = std::clamp(distance, 1.0f, 80.0f); update(); }