#ifndef ASYNC_MESH_PROCESSOR_H
#define ASYNC_MESH_PROCESSOR_H

#include <QObject>
#include <QThread>
#include <QVector3D>
#include <functional>
#include <vector>
#include <memory>

/* --------------------------------------------------------------------------
 * MeshProcessWorker
 * 说明: 在独立线程中执行耗时网格处理，完成后通过信号返回结果。
 * -------------------------------------------------------------------------- */
class MeshProcessWorker : public QObject {
    Q_OBJECT
public:
    using ProcessFunction = std::function<std::pair<std::vector<QVector3D>, std::vector<unsigned int>>(
        const std::vector<QVector3D>&,
        const std::vector<unsigned int>&)>;

    explicit MeshProcessWorker(QObject* parent = nullptr);
    ~MeshProcessWorker();

    void setProcessFunction(ProcessFunction func);

public slots:
    void process(const std::vector<QVector3D>& vertices,
                 const std::vector<unsigned int>& indices);

signals:
    void finished(const std::vector<QVector3D>& vertices,
                  const std::vector<unsigned int>& indices);
    void error(const QString& errorMessage);
    void progressUpdated(int progress); // 0-100

private:
    ProcessFunction processFunc;
};

/* --------------------------------------------------------------------------
 * AsyncMeshProcessor
 * 说明: 封装 worker 线程生命周期，提供 startProcessing 调用接口。
 * -------------------------------------------------------------------------- */
class AsyncMeshProcessor : public QObject {
    Q_OBJECT
public:
    using ProcessFunction = MeshProcessWorker::ProcessFunction;

    explicit AsyncMeshProcessor(QObject* parent = nullptr);
    ~AsyncMeshProcessor();

    void setProcessFunction(ProcessFunction func);
    void startProcessing(const std::vector<QVector3D>& vertices,
                         const std::vector<unsigned int>& indices);
    bool isProcessing() const;
    void cancel(); // 当前未真正实现中断，仅示例扩展位

signals:
    void processingFinished(const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices);
    void processingStarted();
    void processingError(const QString& errorMessage);
    void progressUpdated(int progress);

private slots:
    void onWorkerFinished(const std::vector<QVector3D>& vertices,
                          const std::vector<unsigned int>& indices);
    void onWorkerError(const QString& errorMessage);
    void onWorkerProgress(int progress);

private:
    void startProcessInternal(const std::vector<QVector3D>& vertices,
                              const std::vector<unsigned int>& indices);

    QThread* workerThread { nullptr };
    MeshProcessWorker* worker { nullptr };
    bool processing { false };
};

#endif // ASYNC_MESH_PROCESSOR_H
