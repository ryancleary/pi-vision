#ifndef PIVISION_PIPELINE_FRAMEMETRICS_H
#define PIVISION_PIPELINE_FRAMEMETRICS_H

#include <chrono>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include <pivision/pipeline/DetectionResult.h>
#include <pivision/pipeline/LatestFrameBuffer.h>
#include <pivision/processing/StageChain.h>

namespace pivision::pipeline {

// Frame counts dropped so far at each hand-off. Running totals, never reset.
struct DropCounts {
    std::uint64_t beforeProcessing = 0; // capture -> processing
    std::uint64_t beforeDisplay = 0;    // processing -> GUI
};

// Averages over the recent window. All zero when no frames arrived in it,
// so a stalled feed reads as 0 fps rather than the last good value.
struct MetricsSnapshot {
    double captureFps = 0.0;     // frames the source produced per second
    double displayFps = 0.0;     // frames that reached the GUI per second
    double latencyMs = 0.0;      // capture to GUI
    double stagesMs = 0.0;       // the whole stage chain
    // Average time per stage, in processing order; enabled stages only.
    std::vector<processing::StageTiming> stages;
    double droppedBeforeProcessingPerSecond = 0.0;
    double droppedBeforeDisplayPerSecond = 0.0;

    // Detection, averaged over a longer window (detections are slow, so one
    // second may hold only one or two). All zero while detection is off.
    double detectionMs = 0.0;          // time per detection
    double detectionsPerSecond = 0.0;  // how often results arrive
    double detectionAgeMs = 0.0;       // how old the newest result's frame is now
};

// Rolling statistics over the frames that reached the GUI in the last
// `window`. Lives on the GUI thread, fed by Pipeline as frames arrive.
class FrameMetrics {
public:
    using Clock = std::chrono::steady_clock;

    // Detection statistics use a longer window than frames; see MetricsSnapshot.
    static constexpr std::chrono::seconds kDetectionWindow { 5 };

    explicit FrameMetrics(Clock::duration window = std::chrono::seconds(1),
        Clock::duration detectionWindow = kDetectionWindow)
        : window_(window)
        , detectionWindow_(detectionWindow)
    {
    }

    // Records `frame`, which reached the GUI at `shown`, with the drop totals at that time.
    void add(const DisplayFrame &frame, Clock::time_point shown, const DropCounts &drops);

    // Records a detection result that arrived at `arrived`.
    void addDetection(const DetectionResult &result, Clock::time_point arrived);

    // Forget everything, e.g. when the source changes.
    void clear()
    {
        samples_.clear();
        detections_.clear();
    }
    // Forget detections only, e.g. when detection is switched off.
    void clearDetections() { detections_.clear(); }

    MetricsSnapshot snapshot(Clock::time_point now) const;

private:
    struct Sample {
        Clock::time_point shown;
        Clock::time_point captured;
        std::uint64_t index = 0;
        std::vector<processing::StageTiming> timings;
        DropCounts drops;
    };

    void fillDetectionStatistics(MetricsSnapshot &result, Clock::time_point now) const;

    struct DetectionSample {
        Clock::time_point arrived;
        Clock::time_point captured;
        double milliseconds = 0.0;
    };

    Clock::duration window_;
    Clock::duration detectionWindow_;
    std::deque<Sample> samples_; // oldest first
    std::deque<DetectionSample> detections_; // oldest first
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_FRAMEMETRICS_H
