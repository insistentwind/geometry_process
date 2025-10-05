#ifndef ASYNC_MESH_PROCESSOR_H
#define ASYNC_MESH_PROCESSOR_H

#include <QObject>
#include <QThread>
#include <QVector3D>
#include <functional>
#include <vector>
#include <memory>

/**
 * @brief 异步网格处理工作线程
 * 在独立线程中执行耗时的网格处理操作
 */
class MeshProcessWorker : public QObject
{
    Q_OBJECT

public:
    using ProcessFunction = std::function<std::pair<std::vector<QVector3D>, std::vector<unsigned int>>
                                        (const std::vector<QVector3D>&, const std::vector<unsigned int>&)>;

    explicit MeshProcessWorker(QObject *parent = nullptr);
    ~MeshProcessWorker();

    /**
     * @brief 设置处理函数
     * @param func 处理函数，接收顶点和索引，返回处理后的顶点和索引
     */
    void setProcessFunction(ProcessFunction func);

public slots:
    /**
     * @brief 开始处理网格数据
     * @param vertices 输入顶点数据
     * @param indices 输入索引数据
     */
    void process(const std::vector<QVector3D>& vertices, 
                const std::vector<unsigned int>& indices);

signals:
    /**
     * @brief 处理完成信号
     * @param vertices 处理后的顶点数据
     * @param indices 处理后的索引数据
     */
    void finished(const std::vector<QVector3D>& vertices,
                 const std::vector<unsigned int>& indices);

    /**
     * @brief 处理出错信号
     * @param errorMessage 错误信息
     */
    void error(const QString& errorMessage);

    /**
     * @brief 进度更新信号
     * @param progress 进度百分比 (0-100)
     */
    void progressUpdated(int progress);

private:
    ProcessFunction processFunc;
};

/**
 * @brief 异步网格处理器
 * 管理工作线程，提供简洁的异步处理接口
 * 
 * 使用示例:
 * AsyncMeshProcessor* processor = new AsyncMeshProcessor(this);
 * 
 * // 设置处理函数
 * processor->setProcessFunction([&](const auto& vertices, const auto& indices) {
 *     // 执行耗时操作
 *     return std::make_pair(processedVertices, processedIndices);
 * });
 * 
 * // 连接完成信号
 * connect(processor, &AsyncMeshProcessor::processingFinished, 
 *         this, &MainWindow::onProcessingFinished);
 * 
 * // 开始异步处理
 * processor->startProcessing(vertices, indices);
 */
class AsyncMeshProcessor : public QObject
{
    Q_OBJECT

public:
    using ProcessFunction = MeshProcessWorker::ProcessFunction;

    explicit AsyncMeshProcessor(QObject *parent = nullptr);
    ~AsyncMeshProcessor();

    /**
     * @brief 设置处理函数
     * @param func 处理函数
     */
    void setProcessFunction(ProcessFunction func);

    /**
     * @brief 开始异步处理
     * @param vertices 输入顶点数据
     * @param indices 输入索引数据
     */
    void startProcessing(const std::vector<QVector3D>& vertices,
                        const std::vector<unsigned int>& indices);

    /**
     * @brief 检查是否正在处理
     * @return true表示正在处理中
     */
    bool isProcessing() const;

    /**
     * @brief 取消当前处理（如果正在处理中）
     */
    void cancel();

signals:
    /**
     * @brief 处理完成信号
     * @param vertices 处理后的顶点数据
     * @param indices 处理后的索引数据
     */
    void processingFinished(const std::vector<QVector3D>& vertices,
                          const std::vector<unsigned int>& indices);

    /**
     * @brief 处理开始信号
     */
    void processingStarted();

    /**
     * @brief 处理出错信号
     * @param errorMessage 错误信息
     */
    void processingError(const QString& errorMessage);

    /**
     * @brief 进度更新信号
     * @param progress 进度百分比 (0-100)
     */
    void progressUpdated(int progress);

private slots:
    void onWorkerFinished(const std::vector<QVector3D>& vertices,
                         const std::vector<unsigned int>& indices);
    void onWorkerError(const QString& errorMessage);
    void onWorkerProgress(int progress);

private:
    void startProcessInternal(const std::vector<QVector3D>& vertices,
                            const std::vector<unsigned int>& indices);

    QThread* workerThread;
    MeshProcessWorker* worker;
    bool processing;
};

#endif // ASYNC_MESH_PROCESSOR_H
