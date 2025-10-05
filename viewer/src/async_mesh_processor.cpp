#include "async_mesh_processor.h"
#include <QDebug>

// ============================================================================
// MeshProcessWorker Implementation
// ============================================================================

MeshProcessWorker::MeshProcessWorker(QObject *parent)
    : QObject(parent)
{
}

MeshProcessWorker::~MeshProcessWorker()
{
}

void MeshProcessWorker::setProcessFunction(ProcessFunction func)
{
    processFunc = func;
}

void MeshProcessWorker::process(const std::vector<QVector3D>& vertices,
                               const std::vector<unsigned int>& indices)
{
    try {
        if (!processFunc) {
            emit error("Process function not set");
            return;
        }

        qDebug() << "Worker thread: Starting mesh processing...";
        emit progressUpdated(0);

        // 执行处理函数
        auto result = processFunc(vertices, indices);

        emit progressUpdated(100);
        qDebug() << "Worker thread: Mesh processing completed.";

        // 发送结果
        emit finished(result.first, result.second);

    } catch (const std::exception& e) {
        emit error(QString("Processing error: %1").arg(e.what()));
    } catch (...) {
        emit error("Unknown processing error");
    }
}

// ============================================================================
// AsyncMeshProcessor Implementation
// ============================================================================

AsyncMeshProcessor::AsyncMeshProcessor(QObject *parent)
    : QObject(parent)
    , workerThread(nullptr)
    , worker(nullptr)
    , processing(false)
{
    // 创建工作线程
    workerThread = new QThread(this);

    // 创建工作对象
    worker = new MeshProcessWorker();
    worker->moveToThread(workerThread);

    // 连接信号和槽
    connect(worker, &MeshProcessWorker::finished,
            this, &AsyncMeshProcessor::onWorkerFinished);
    connect(worker, &MeshProcessWorker::error,
            this, &AsyncMeshProcessor::onWorkerError);
    connect(worker, &MeshProcessWorker::progressUpdated,
            this, &AsyncMeshProcessor::onWorkerProgress);

    // 线程清理
    connect(workerThread, &QThread::finished,
            worker, &QObject::deleteLater);

    // 启动线程
    workerThread->start();
}

AsyncMeshProcessor::~AsyncMeshProcessor()
{
    // 等待线程结束
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
    }
}

void AsyncMeshProcessor::setProcessFunction(ProcessFunction func)
{
    worker->setProcessFunction(func);
}

void AsyncMeshProcessor::startProcessing(const std::vector<QVector3D>& vertices,
                                        const std::vector<unsigned int>& indices)
{
    if (processing) {
        qWarning() << "AsyncMeshProcessor: Already processing, ignoring new request";
        return;
    }

    processing = true;
    emit processingStarted();
    qDebug() << "AsyncMeshProcessor: Starting async processing...";

    // 使用QMetaObject::invokeMethod来确保在工作线程中执行
    QMetaObject::invokeMethod(worker, 
        [this, vertices, indices]() {
            worker->process(vertices, indices);
        },
        Qt::QueuedConnection);
}

bool AsyncMeshProcessor::isProcessing() const
{
    return processing;
}

void AsyncMeshProcessor::cancel()
{
    if (processing) {
        // 注意：这个简单实现不会真正中断正在执行的操作
        // 如果需要真正的取消功能，需要在处理函数中添加取消检查点
        qDebug() << "AsyncMeshProcessor: Cancel requested (processing will complete)";
    }
}

void AsyncMeshProcessor::onWorkerFinished(const std::vector<QVector3D>& vertices,
                                         const std::vector<unsigned int>& indices)
{
    processing = false;
    qDebug() << "AsyncMeshProcessor: Processing finished successfully";
    emit processingFinished(vertices, indices);
}

void AsyncMeshProcessor::onWorkerError(const QString& errorMessage)
{
    processing = false;
    qWarning() << "AsyncMeshProcessor: Processing error:" << errorMessage;
    emit processingError(errorMessage);
}

void AsyncMeshProcessor::onWorkerProgress(int progress)
{
    emit progressUpdated(progress);
}
