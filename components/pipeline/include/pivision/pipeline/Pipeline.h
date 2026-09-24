#ifndef PIVISION_PIPELINE_PIPELINE_H
#define PIVISION_PIPELINE_PIPELINE_H

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#include <QList>
#include <QObject>
#include <QSize>
#include <QString>
#include <QThread>

#include <pivision/capture/FrameSource.h>
#include <pivision/pipeline/DetectionResult.h>
#include <pivision/pipeline/FrameMetrics.h>
#include <pivision/pipeline/LatestFrameBuffer.h>
#include <pivision/pipeline/StageConfig.h>

namespace pivision::pipeline {

class DetectionWorker;
class FrameProcessor;
class FrameProducer;
struct DetectionJob;
struct RawFrame;

// Capture, processing and detection each run on their own thread:
//   capture thread     reads the source            -> newest raw frame
//   processing thread  scales it, runs the stages  -> newest display frame
//                      (and, while detection is on, -> newest detection job)
//   detection thread   runs the detector           -> newest detection result
//   GUI thread         latestFrame() after frameAvailable(),
//                      latestDetection() after detectionAvailable()
// Each hand-off keeps only the newest frame, so a slow step drops frames rather
// than falling behind, and capture never waits for processing.
// Lives on the GUI thread.
class Pipeline : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString sourceName READ sourceName NOTIFY sourceNameChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
public:
    explicit Pipeline(std::unique_ptr<capture::FrameSource> source, QObject *parent = nullptr);
    ~Pipeline() override;

    void start();
    void stop();

    // Switches to `source`. The current feed stops immediately (cleared() is
    // emitted); if the new source fails to open, failed() reports why and no
    // frames arrive until another source is set.
    void setSource(std::unique_ptr<capture::FrameSource> source);

    // Processing settings. Safe to call before or after start().
    void setStages(const QList<StageConfig> &stages);
    void setStageEnabled(const QString &id, bool enabled);
    void setStageParameter(const QString &id, const QString &name, double value);
    void setProcessSize(const QSize &size);
    QSize processSize() const { return processSize_; }

    // Object detection. Off until enabled; the model loads the first time it
    // is enabled. Detection is slower than the frame rate, so it works on the
    // newest frame whenever it's free and skips the rest. Safe to call before
    // or after start(). Set the model description before enabling.
    void setModelDescription(const QString &path);
    void setDetectionEnabled(bool enabled);
    bool detectionEnabled() const { return detectionEnabled_; }
    void setDetectionInput(DetectionInput input);
    DetectionInput detectionInput() const { return detectionInput_; }

    // The newest detection for the current source and input; empty while
    // detection is off, after a switch, and before the first result.
    std::optional<DetectionResult> latestDetection() const { return latestDetection_; }
    DetectorState detectorState() const { return detectorState_; }
    // Why the model couldn't be loaded; empty unless the state is Failed.
    QString detectorError() const { return detectorError_; }

    // The newest frame; empty before the first one and after a switch or failure.
    // Readers share it (QImage is implicitly shared), so any number of views can.
    std::optional<DisplayFrame> latestFrame() const { return latest_; }

    // Frames dropped because the next step was still busy. Defined in the .cpp:
    // they need RawFrame, which is private to this component.
    std::uint64_t droppedFrames() const;
    DropCounts dropCounts() const;

    // Frame rates, latency and stage times over the last second. Reset when
    // the source changes or fails.
    MetricsSnapshot metrics() const { return metrics_.snapshot(FrameMetrics::Clock::now()); }
    QString sourceName() const { return sourceName_; }
    // Why the current source isn't producing frames; empty while it is.
    QString errorString() const { return errorString_; }

signals:
    void frameAvailable();
    void failed(const QString &message);
    // Whatever is on screen is stale and should be removed.
    void cleared();
    void sourceNameChanged();
    void errorStringChanged();
    void detectionAvailable();
    void detectorStateChanged();

private:
    void onProcessedFrame();
    void onDetectionResult();
    void onDetectorState(DetectorState state, const QString &error);
    // Tells the processing thread what to hand the detector.
    void updateDetectionFeed();
    void clearDetection();
    void onOpened(const QString &name);
    void onFailed(const QString &message);
    void setSourceName(const QString &name);
    void setErrorString(const QString &message);
    // Runs `task` on the processing thread (or right away if it isn't running).
    void onProcessingThread(std::function<void(FrameProcessor &)> task);
    // The same for the detection thread.
    void onDetectionThread(std::function<void(DetectionWorker &)> task);

    // Order matters: members are destroyed bottom-up, so the threads stop
    // before the workers go, and the workers before the buffers they use.
    std::unique_ptr<LatestBuffer<RawFrame>> rawBuffer_;
    LatestFrameBuffer displayBuffer_;
    std::unique_ptr<LatestBuffer<DetectionJob>> detectionJobs_;
    std::unique_ptr<LatestBuffer<DetectionResult>> detectionResults_;
    std::unique_ptr<FrameProducer> producer_;
    std::unique_ptr<FrameProcessor> processor_;
    std::unique_ptr<DetectionWorker> detector_;
    QThread captureThread_;
    QThread processingThread_;
    QThread detectionThread_;

    std::optional<DisplayFrame> latest_;
    std::optional<DetectionResult> latestDetection_;
    bool detectionEnabled_ = false;
    DetectionInput detectionInput_ = DetectionInput::Raw;
    DetectorState detectorState_ = DetectorState::Off;
    QString detectorError_;
    FrameMetrics metrics_;
    std::uint64_t generation_ = 0;
    QSize processSize_;
    QString sourceName_;
    QString errorString_;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_PIPELINE_H
