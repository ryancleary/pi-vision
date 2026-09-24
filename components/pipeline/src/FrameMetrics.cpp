#include <pivision/pipeline/FrameMetrics.h>

#include <algorithm>
#include <iterator>
#include <numeric>

#include "Utils.h"

namespace pivision::pipeline {

void FrameMetrics::add(const DisplayFrame &frame, Clock::time_point shown, const DropCounts &drops)
{
    samples_.push_back(Sample { shown, frame.captured, frame.index, frame.timings, drops });
    // Keep only what snapshot() can still use.
    while (!samples_.empty() && samples_.front().shown < shown - window_)
        samples_.pop_front();
}

void FrameMetrics::addDetection(const DetectionResult &result, Clock::time_point arrived)
{
    detections_.push_back(DetectionSample { arrived, result.captured, result.milliseconds });
    while (!detections_.empty() && detections_.front().arrived < arrived - detectionWindow_)
        detections_.pop_front();
}

void FrameMetrics::fillDetectionStatistics(MetricsSnapshot &result, Clock::time_point now) const
{
    const auto first = std::find_if(detections_.begin(), detections_.end(),
        [this, now](const DetectionSample &sample) { return sample.arrived >= now - detectionWindow_; });
    const auto count = std::distance(first, detections_.end());
    if (count == 0)
        return;

    const double totalMs = std::accumulate(first, detections_.end(), 0.0,
        [](double total, const DetectionSample &sample) { return total + sample.milliseconds; });
    result.detectionMs = totalMs / static_cast<double>(count);
    if (count >= 2) {
        result.detectionsPerSecond
            = ratePerSecond(static_cast<double>(count - 1), detections_.back().arrived - first->arrived);
    }
    // How long ago the frame behind the newest boxes was captured: the delay
    // between the live picture and the boxes drawn on it.
    result.detectionAgeMs
        = std::chrono::duration<double, std::milli>(now - detections_.back().captured).count();
}

MetricsSnapshot FrameMetrics::snapshot(Clock::time_point now) const
{
    // Samples inside the window; they are in time order, so it's a suffix.
    const auto first = std::find_if(samples_.begin(), samples_.end(),
        [this, now](const Sample &sample) { return sample.shown >= now - window_; });
    const auto count = std::distance(first, samples_.end());

    MetricsSnapshot result;
    fillDetectionStatistics(result, now);
    if (count == 0)
        return result;

    // Rates come from the first and last sample in the window: the change in a
    // count divided by the time between them. Needs at least two samples.
    const Sample &oldest = *first;
    const Sample &newest = samples_.back();
    if (count >= 2) {
        result.displayFps = ratePerSecond(static_cast<double>(count - 1), newest.shown - oldest.shown);
        // The source numbers every frame it reads, including ones dropped later.
        result.captureFps = ratePerSecond(
            static_cast<double>(newest.index - oldest.index), newest.captured - oldest.captured);
        result.droppedBeforeProcessingPerSecond = ratePerSecond(
            static_cast<double>(newest.drops.beforeProcessing - oldest.drops.beforeProcessing),
            newest.shown - oldest.shown);
        result.droppedBeforeDisplayPerSecond = ratePerSecond(
            static_cast<double>(newest.drops.beforeDisplay - oldest.drops.beforeDisplay),
            newest.shown - oldest.shown);
    }

    const double latencyTotalMs = std::accumulate(first, samples_.end(), 0.0,
        [](double total, const Sample &sample) {
            return total + std::chrono::duration<double, std::milli>(sample.shown - sample.captured).count();
        });
    result.latencyMs = latencyTotalMs / static_cast<double>(count);

    // Average each stage over the frames it ran on, keeping the order the
    // stages first appear in (the processing order).
    std::vector<int> runs;
    for (auto it = first; it != samples_.end(); ++it) {
        for (const processing::StageTiming &timing : it->timings) {
            auto match = std::find_if(result.stages.begin(), result.stages.end(),
                [&timing](const processing::StageTiming &s) { return s.id == timing.id; });
            if (match == result.stages.end()) {
                result.stages.push_back(timing);
                runs.push_back(1);
            } else {
                match->milliseconds += timing.milliseconds;
                ++runs[static_cast<std::size_t>(std::distance(result.stages.begin(), match))];
            }
        }
    }
    for (std::size_t i = 0; i < result.stages.size(); ++i)
        result.stages[i].milliseconds /= runs[i];

    // Chain time per frame, averaged; frames with no stages count as zero.
    const double stagesTotalMs = std::accumulate(first, samples_.end(), 0.0,
        [](double total, const Sample &sample) {
            return std::accumulate(sample.timings.begin(), sample.timings.end(), total,
                [](double sum, const processing::StageTiming &t) { return sum + t.milliseconds; });
        });
    result.stagesMs = stagesTotalMs / static_cast<double>(count);

    return result;
}

} // namespace pivision::pipeline
