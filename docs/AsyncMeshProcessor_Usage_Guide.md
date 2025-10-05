# �첽��������ʹ��ָ��

## ����

`AsyncMeshProcessor` ��һ����װ�� `viewer` ģ���е�ͨ���첽�����ܣ������ڶ����߳���ִ�к�ʱ�����������������Qt��������Ӧ��

## ��Ҫ����

- ? **������UI**: ��ʱ�����ڶ����߳���ִ�У������汣����Ӧ
- ? **ͨ�ýӿ�**: ʹ�ú������󣬿������ɼ����κδ����߼�
- ? **�ź�֪ͨ**: �ṩ����ʼ����ɡ�����ͽ��ȵ��ź�
- ? **�̰߳�ȫ**: �Զ��������̵߳���������
- ? **���ڸ���**: ���ڶ��hw��Ŀ��ʹ��

## ���ٿ�ʼ

### 1. ����ͷ�ļ�

```cpp
#include <async_mesh_processor.h>
#include <your_mesh_processor.h>
```

### 2. �����첽������ʵ��

```cpp
// ��main�������������д���
AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);
```

### 3. ���ô�����

```cpp
YourMeshProcessor processor;  // �����������

asyncProcessor->setProcessFunction(
    [&processor](const std::vector<QVector3D>& vertices,
                const std::vector<unsigned int>& indices) {
        // ����������ڶ����߳���ִ��
        // ���Է��ĵ�ִ�к�ʱ����
        return processor.processOBJData(vertices, indices);
    });
```

### 4. �����ź�

```cpp
// ��ʼ����
QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingStarted,
    []() {
        qDebug() << "Processing started...";
        // ������ʾ���ȶԻ���
    });

// �������
QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
    [&window](const std::vector<QVector3D>& vertices,
             const std::vector<unsigned int>& indices) {
        qDebug() << "Processing completed!";
        window.updateMesh(vertices, indices);
    });

// �������
QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
    [](const QString& errorMessage) {
        qWarning() << "Error:" << errorMessage;
    });

// ���ȸ��£���ѡ��
QObject::connect(asyncProcessor, &AsyncMeshProcessor::progressUpdated,
    [](int progress) {
        qDebug() << "Progress:" << progress << "%";
    });
```

### 5. �����첽����

```cpp
// ������OBJ�ļ�����������
QObject::connect(&window, &MainWindow::objLoaded,
    [asyncProcessor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
        asyncProcessor->startProcessing(vertices, indices);
    });
```

## ����ʾ�� (hw1)

```cpp
#include <QApplication>
#include <QMessageBox>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <mesh_processor.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    // ����������
    MeshProcessor processor;
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

    // ���ô�����
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
            return processor.processOBJData(vertices, indices);
        });

    // ����OBJ�����¼�
    QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor](const std::vector<QVector3D>& vertices,
                        const std::vector<unsigned int>& indices) {
            asyncProcessor->startProcessing(vertices, indices);
        });

    // ��������¼�
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window](const std::vector<QVector3D>& vertices,
                 const std::vector<unsigned int>& indices) {
            window.updateMesh(vertices, indices);
        });

    // ���Ӵ�����
    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingError,
        [&window](const QString& errorMessage) {
            QMessageBox::warning(&window, "Error", errorMessage);
        });

    window.show();
    return app.exec();
}
```

## ���µ�hw��Ŀ��ʹ��

### hw2 ʾ��

```cpp
// src/hw2/src/hw2.cpp
#include <QApplication>
#include <mainwindow.h>
#include <async_mesh_processor.h>
#include <hw2_mesh_processor.h>  // ���hw2������

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.resize(800, 600);

    HW2MeshProcessor processor;  // ���hw2�ض�������
    AsyncMeshProcessor* asyncProcessor = new AsyncMeshProcessor(&window);

    // ����hw2�Ĵ����߼�
    asyncProcessor->setProcessFunction(
        [&processor](const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices) {
            // hw2�ض��Ĵ����߼�
            return processor.doHW2Processing(vertices, indices);
        });

    // �������Ӵ�����ͬ...
    QObject::connect(&window, &MainWindow::objLoaded,
        [asyncProcessor](const std::vector<QVector3D>& vertices,
                        const std::vector<unsigned int>& indices) {
            asyncProcessor->startProcessing(vertices, indices);
        });

    QObject::connect(asyncProcessor, &AsyncMeshProcessor::processingFinished,
        [&window](const auto& v, const auto& i) {
            window.updateMesh(v, i);
        });

    window.show();
    return app.exec();
}
```

## API �ο�

### AsyncMeshProcessor

#### ���캯��
```cpp
AsyncMeshProcessor(QObject *parent = nullptr)
```

#### ��Ա����

```cpp
// ���ô�����
void setProcessFunction(ProcessFunction func)

// ��ʼ�첽����
void startProcessing(const std::vector<QVector3D>& vertices,
                    const std::vector<unsigned int>& indices)

// ����Ƿ����ڴ���
bool isProcessing() const

// ȡ������ע�⣺��ǰʵ�ֲ����ж�����ִ�еĲ�����
void cancel()
```

#### �ź�

```cpp
// ����ʼ
void processingStarted()

// �������
void processingFinished(const std::vector<QVector3D>& vertices,
                       const std::vector<unsigned int>& indices)

// �������
void processingError(const QString& errorMessage)

// ���ȸ���
void progressUpdated(int progress)
```

## ע������

1. **�̰߳�ȫ**: �������в�Ҫֱ�Ӳ���Qt UI�����ʹ���ź�-�ۻ���
2. **�ڴ����**: AsyncMeshProcessorʹ�ø���������������ڣ�ͨ����MainWindow��Ϊ������
3. **�ظ�����**: ������ڴ����У��ٴε���`startProcessing`�ᱻ����
4. **�쳣����**: �������е��쳣�ᱻ����ͨ��`processingError`�źŷ���

## ��ӽ��ȱ���

�����Ҫ�ڴ�������б�����ȣ������ڴ��������з��ͽ����źţ�

```cpp
// ����Ĵ���������
class MyProcessor {
public:
    std::pair<std::vector<QVector3D>, std::vector<unsigned int>> 
    processOBJData(const std::vector<QVector3D>& vertices,
                   const std::vector<unsigned int>& indices) {
        // ������1 - 30%
        doStep1();
        // ��Ҫͨ���ص����������Ʊ������
        
        // ������2 - 60%
        doStep2();
        
        // ������3 - 100%
        doStep3();
        
        return result;
    }
};
```

## �����ų�

### ���⣺������Ȼ����Ӧ
- ȷ�ϴ�����������첽��������ִ�У�������ֱ�ӵ���
- ����Ƿ��ڴ������е�����������UI����

### ���⣺������û�и��µ�����
- ȷ��������`processingFinished`�ź�
- ���`updateMesh`�Ƿ���ȷ����

### ���⣺��������Ҳ���async_mesh_processor.h
- ȷ��viewer������ȷ���ӵ����hw��Ŀ
- ���CMakeLists.txt���Ƿ����viewer����

## ��չ�Ķ�

- Qt�̺߳�QObject: https://doc.qt.io/qt-6/thread-basics.html
- �ź���ۻ���: https://doc.qt.io/qt-6/signalsandslots.html
