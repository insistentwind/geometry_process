#include "async_mesh_processor.h"
#include <QDebug>

/* ============================ MeshProcessWorker ============================ */
MeshProcessWorker::MeshProcessWorker(QObject *parent) : QObject(parent) {}
MeshProcessWorker::~MeshProcessWorker() = default;

void MeshProcessWorker::setProcessFunction(ProcessFunction func) { processFunc = std::move(func); }

void MeshProcessWorker::process(const std::vector<QVector3D>& vertices,
                                const std::vector<unsigned int>& indices) {
    try {
        if (!processFunc) {
            emit error("Process function not set");
            return;
        }
        qDebug() << "Worker thread: Starting mesh processing...";
        emit progressUpdated(0);
        auto result = processFunc(vertices, indices); // 执行耗时任务
        emit progressUpdated(100);
        qDebug() << "Worker thread: Mesh processing completed.";
        emit finished(result.first, result.second);
    } catch (const std::exception& e) {
        emit error(QString("Processing error: %1").arg(e.what()));
    } catch (...) {
        emit error("Unknown processing error");
    }
}

/* =========================== AsyncMeshProcessor ============================ */
AsyncMeshProcessor::AsyncMeshProcessor(QObject *parent)
    : QObject(parent) {
    workerThread = new QThread(this);
    worker = new MeshProcessWorker();
    worker->moveToThread(workerThread);

    connect(worker, &MeshProcessWorker::finished,
            this, &AsyncMeshProcessor::onWorkerFinished);
    connect(worker, &MeshProcessWorker::error,
            this, &AsyncMeshProcessor::onWorkerError);
    connect(worker, &MeshProcessWorker::progressUpdated,
            this, &AsyncMeshProcessor::onWorkerProgress);

    connect(workerThread, &QThread::finished,
            worker, &QObject::deleteLater);

    workerThread->start();
}

AsyncMeshProcessor::~AsyncMeshProcessor() {
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
    }
}

void AsyncMeshProcessor::setProcessFunction(ProcessFunction func) { worker->setProcessFunction(std::move(func)); }

void AsyncMeshProcessor::startProcessing(const std::vector<QVector3D>& vertices,
                                         const std::vector<unsigned int>& indices) {
    if (processing) {
        qWarning() << "AsyncMeshProcessor: Already processing, ignoring new request";
        return;
    }
    processing = true;
    emit processingStarted();
    qDebug() << "AsyncMeshProcessor: Starting async processing...";

    QMetaObject::invokeMethod(worker, [this, vertices, indices]() {
        worker->process(vertices, indices);
    }, Qt::QueuedConnection);
}

bool AsyncMeshProcessor::isProcessing() const { return processing; }

void AsyncMeshProcessor::cancel() {
    if (processing) {
        qDebug() << "AsyncMeshProcessor: Cancel requested (not implemented)";
    }
}

void AsyncMeshProcessor::onWorkerFinished(const std::vector<QVector3D>& vertices,
                                          const std::vector<unsigned int>& indices) {
    processing = false;
    qDebug() << "AsyncMeshProcessor: Processing finished successfully";
    emit processingFinished(vertices, indices);
}

void AsyncMeshProcessor::onWorkerError(const QString& errorMessage) {
    processing = false;
    qWarning() << "AsyncMeshProcessor: Processing error:" << errorMessage;
    emit processingError(errorMessage);
}

void AsyncMeshProcessor::onWorkerProgress(int progress) { emit progressUpdated(progress); }
