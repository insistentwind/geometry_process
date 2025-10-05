#ifndef ASYNC_MESH_PROCESSOR_H
#define ASYNC_MESH_PROCESSOR_H

#include <QObject>
#include <QThread>
#include <QVector3D>
#include <functional>
#include <vector>
#include <memory>

/**
 * @brief �첽���������߳�
 * �ڶ����߳���ִ�к�ʱ�����������
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
     * @brief ���ô�����
     * @param func �����������ն�������������ش����Ķ��������
     */
    void setProcessFunction(ProcessFunction func);

public slots:
    /**
     * @brief ��ʼ������������
     * @param vertices ���붥������
     * @param indices ������������
     */
    void process(const std::vector<QVector3D>& vertices, 
                const std::vector<unsigned int>& indices);

signals:
    /**
     * @brief ��������ź�
     * @param vertices �����Ķ�������
     * @param indices ��������������
     */
    void finished(const std::vector<QVector3D>& vertices,
                 const std::vector<unsigned int>& indices);

    /**
     * @brief ��������ź�
     * @param errorMessage ������Ϣ
     */
    void error(const QString& errorMessage);

    /**
     * @brief ���ȸ����ź�
     * @param progress ���Ȱٷֱ� (0-100)
     */
    void progressUpdated(int progress);

private:
    ProcessFunction processFunc;
};

/**
 * @brief �첽��������
 * �������̣߳��ṩ�����첽����ӿ�
 * 
 * ʹ��ʾ��:
 * AsyncMeshProcessor* processor = new AsyncMeshProcessor(this);
 * 
 * // ���ô�����
 * processor->setProcessFunction([&](const auto& vertices, const auto& indices) {
 *     // ִ�к�ʱ����
 *     return std::make_pair(processedVertices, processedIndices);
 * });
 * 
 * // ��������ź�
 * connect(processor, &AsyncMeshProcessor::processingFinished, 
 *         this, &MainWindow::onProcessingFinished);
 * 
 * // ��ʼ�첽����
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
     * @brief ���ô�����
     * @param func ������
     */
    void setProcessFunction(ProcessFunction func);

    /**
     * @brief ��ʼ�첽����
     * @param vertices ���붥������
     * @param indices ������������
     */
    void startProcessing(const std::vector<QVector3D>& vertices,
                        const std::vector<unsigned int>& indices);

    /**
     * @brief ����Ƿ����ڴ���
     * @return true��ʾ���ڴ�����
     */
    bool isProcessing() const;

    /**
     * @brief ȡ����ǰ����������ڴ����У�
     */
    void cancel();

signals:
    /**
     * @brief ��������ź�
     * @param vertices �����Ķ�������
     * @param indices ��������������
     */
    void processingFinished(const std::vector<QVector3D>& vertices,
                          const std::vector<unsigned int>& indices);

    /**
     * @brief ����ʼ�ź�
     */
    void processingStarted();

    /**
     * @brief ��������ź�
     * @param errorMessage ������Ϣ
     */
    void processingError(const QString& errorMessage);

    /**
     * @brief ���ȸ����ź�
     * @param progress ���Ȱٷֱ� (0-100)
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
