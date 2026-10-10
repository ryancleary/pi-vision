#include <pivision/pipeline/Pipeline.h>

#include <utility>

#include <pivision/logging/Logging.h>

#include "DetectionJob.h"
#include "DetectionWorker.h"
#include "FrameProcessor.h"
#include "FrameProducer.h"
#include "RawFrame.h"

namespace pivision::pipeline {

Pipeline::Pipeline(std::unique_ptr<capture::FrameSource> source, QObject *parent)
    : QObject(parent)
    , rawBuffer_(std::make_unique<RawFrameBuffer>())
    , detectionJobs_(std::make_unique<DetectionJobBuffer>())
    , detectionResults_(std::make_unique<DetectionResultBuffer>())
    , sourceName_(QString::fromStdString(source->name()))
{
    producer_ = std::make_unique<FrameProducer>(std::move(source), *rawBuffer_);
    processor_ = std::make_unique<FrameProcessor>(*rawBuffer_, displayBuffer_, *detectionJobs_);
    detector_ = std::make_unique<DetectionWorker>(*detectionJobs_, *detectionResults_);
    producer_->moveToThread(&captureThread_);
    processor_->moveToThread(&processingThread_);
    detector_->moveToThread(&detectionThread_);
    captureThread_.setObjectName(QStringLiteral("capture"));
    processingThread_.setObjectName(QStringLiteral("processing"));
    detectionThread_.setObjectName(QStringLiteral("detection"));

    connect(&captureThread_, &QThread::started, producer_.get(), &FrameProducer::start);
    // Each arrow crosses a thread, so these connections are queued automatically.
    connect(producer_.get(), &FrameProducer::frameAvailable, processor_.get(),
        &FrameProcessor::processLatest);
    connect(processor_.get(), &FrameProcessor::frameAvailable, this, &Pipeline::onProcessedFrame);
    connect(producer_.get(), &FrameProducer::opened, this, &Pipeline::onOpened);
    connect(producer_.get(), &FrameProducer::failed, this, &Pipeline::onFailed);
    connect(processor_.get(), &FrameProcessor::detectionJobAvailable, detector_.get(),
        &DetectionWorker::detectLatest);
    connect(detector_.get(), &DetectionWorker::resultAvailable, this, &Pipeline::onDetectionResult);
    connect(detector_.get(), &DetectionWorker::stateChanged, this, &Pipeline::onDetectorState);
}

Pipeline::~Pipeline() { stop(); }

void Pipeline::start()
{
    if (captureThread_.isRunning())
        return;
    qCDebug(logging::lcPipeline) << "Starting capture, processing and detection threads";
    // Downstream first, so each step is ready for the first frame.
    detectionThread_.start();
    processingThread_.start();
    captureThread_.start();
}

void Pipeline::stop()
{
    if (captureThread_.isRunning()) {
        qCDebug(logging::lcPipeline) << "Stopping; dropped frames:" << droppedFrames();
        // Run stop() on the producer's own thread and wait for it, so the timer
        // and source are shut down by the thread that owns them.
        QMetaObject::invokeMethod(
            producer_.get(), &FrameProducer::stop, Qt::BlockingQueuedConnection);
        captureThread_.quit();
        captureThread_.wait();
    }
    if (processingThread_.isRunning()) {
        processingThread_.quit();
        processingThread_.wait();
    }
    // A detection in progress finishes first (it can't be interrupted); on the
    // Pi that's up to about half a second.
    if (detectionThread_.isRunning()) {
        detectionThread_.quit();
        detectionThread_.wait();
    }
}

void Pipeline::setSource(std::unique_ptr<capture::FrameSource> source)
{
    if (!source)
        return;

    qCInfo(logging::lcPipeline) << "Switching source to" << QString::fromStdString(source->name());
    // Cut the old feed right away; the new one appears once it opens. Frames
    // still in flight from the old source carry the old generation and are dropped.
    ++generation_;
    latest_.reset();
    metrics_.clear();
    clearDetection();
    setSourceName(QString::fromStdString(source->name()));
    setErrorString({});
    emit cleared();

    if (!captureThread_.isRunning()) {
        // Nothing runs on the capture thread yet, so it's safe to call directly.
        producer_->replaceSource(std::move(source));
        return;
    }

    // Opening can be slow (cameras), so it happens on the capture thread. The
    // shared holder frees the source even if the call never runs, for example
    // when the pipeline stops before the queued call is processed.
    auto holder = std::make_shared<std::unique_ptr<capture::FrameSource>>(std::move(source));
    FrameProducer *producer = producer_.get();
    QMetaObject::invokeMethod(
        producer, [producer, holder] { producer->replaceSource(std::move(*holder)); },
        Qt::QueuedConnection);
}

void Pipeline::setStages(const QList<StageConfig> &stages)
{
    onProcessingThread([stages](FrameProcessor &processor) { processor.configure(stages); });
}

void Pipeline::setStageEnabled(const QString &id, bool enabled)
{
    qCDebug(logging::lcPipeline) << "Stage" << id << (enabled ? "on" : "off");
    onProcessingThread([id = id.toStdString(), enabled](FrameProcessor &processor) {
        processor.setStageEnabled(id, enabled);
    });
}

void Pipeline::setStageParameter(const QString &id, const QString &name, double value)
{
    onProcessingThread([id = id.toStdString(), name = name.toStdString(), value](
                           FrameProcessor &processor) { processor.setStageParameter(id, name, value); });
}

void Pipeline::setProcessSize(const QSize &size)
{
    processSize_ = size;
    onProcessingThread([size](FrameProcessor &processor) { processor.setProcessSize(size); });
}

std::uint64_t Pipeline::droppedFrames() const
{
    const DropCounts drops = dropCounts();
    return drops.beforeProcessing + drops.beforeDisplay;
}

DropCounts Pipeline::dropCounts() const
{
    return DropCounts { rawBuffer_->droppedCount(), displayBuffer_.droppedCount() };
}

void Pipeline::onProcessedFrame()
{
    auto frame = displayBuffer_.take();
    if (!frame || frame->generation != generation_)
        return; // already taken, or from a source that has since been replaced
    latest_ = std::move(frame);
    metrics_.add(*latest_, FrameMetrics::Clock::now(), dropCounts());
    emit frameAvailable();
}

void Pipeline::onOpened(const QString &name)
{
    setSourceName(name);
    setErrorString({});
}

void Pipeline::onFailed(const QString &message)
{
    latest_.reset();
    metrics_.clear();
    clearDetection();
    setErrorString(message);
    emit failed(message);
}

void Pipeline::setSourceName(const QString &name)
{
    if (name == sourceName_)
        return;
    sourceName_ = name;
    emit sourceNameChanged();
}

void Pipeline::setErrorString(const QString &message)
{
    if (message == errorString_)
        return;
    errorString_ = message;
    emit errorStringChanged();
}

void Pipeline::setModelDescription(const QString &path)
{
    onDetectionThread([path = path.toStdString()](DetectionWorker &worker) {
        worker.setModelDescription(path);
    });
}

void Pipeline::setDetectionEnabled(bool enabled)
{
    if (enabled == detectionEnabled_)
        return;
    qCInfo(logging::lcPipeline) << "Detection" << (enabled ? "on" : "off");
    detectionEnabled_ = enabled;
    if (!enabled)
        clearDetection();
    updateDetectionFeed();
}

void Pipeline::setDetectionInput(DetectionInput input)
{
    if (input == detectionInput_)
        return;
    detectionInput_ = input;
    clearDetection(); // boxes from the other image don't fit this one
    updateDetectionFeed();
}

void Pipeline::updateDetectionFeed()
{
    onProcessingThread([enabled = detectionEnabled_, input = detectionInput_](FrameProcessor &processor) {
        processor.setDetectionFeed(enabled, input);
    });
}

void Pipeline::clearDetection()
{
    // Results still on their way are recognized as stale in onDetectionResult().
    latestDetection_.reset();
    metrics_.clearDetections();
    emit detectionAvailable();
}

void Pipeline::onDetectionResult()
{
    auto result = detectionResults_->take();
    // Drop results for a replaced source, the other input, or after switching off.
    if (!result || !detectionEnabled_ || result->generation != generation_
        || result->input != detectionInput_)
        return;
    latestDetection_ = std::move(result);
    metrics_.addDetection(*latestDetection_, FrameMetrics::Clock::now());
    emit detectionAvailable();
}

void Pipeline::onDetectorState(DetectorState state, const QString &error)
{
    detectorState_ = state;
    detectorError_ = error;
    emit detectorStateChanged();
}

void Pipeline::onDetectionThread(std::function<void(DetectionWorker &)> task)
{
    DetectionWorker *worker = detector_.get();
    if (!detectionThread_.isRunning()) {
        task(*worker);
        return;
    }
    QMetaObject::invokeMethod(
        worker, [worker, task = std::move(task)] { task(*worker); }, Qt::QueuedConnection);
}

void Pipeline::onProcessingThread(std::function<void(FrameProcessor &)> task)
{
    FrameProcessor *processor = processor_.get();
    if (!processingThread_.isRunning()) {
        task(*processor);
        return;
    }
    QMetaObject::invokeMethod(
        processor, [processor, task = std::move(task)] { task(*processor); }, Qt::QueuedConnection);
}

} // namespace pivision::pipeline
