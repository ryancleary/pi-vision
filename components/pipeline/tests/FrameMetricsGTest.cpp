#include <gtest/gtest.h>

#include <chrono>

#include <pivision/pipeline/FrameMetrics.h>

namespace pivision::pipeline {

class FrameMetricsTest : public ::testing::Test {
protected:
    // A frame with capture index `index`, captured at `captured`, whose stages took `timings`.
    static DisplayFrame frameAt(std::uint64_t index, FrameMetrics::Clock::time_point captured,
        std::vector<processing::StageTiming> timings = {})
    {
        DisplayFrame frame;
        frame.index = index;
        frame.captured = captured;
        frame.timings = std::move(timings);
        return frame;
    }
};

TEST_F(FrameMetricsTest, GivenNoFrames_WhenTakingASnapshot_ThenEverythingIsZero)
{
    const FrameMetrics metrics;
    const MetricsSnapshot snapshot = metrics.snapshot(FrameMetrics::Clock::now());
    EXPECT_EQ(snapshot.displayFps, 0.0);
    EXPECT_EQ(snapshot.captureFps, 0.0);
    EXPECT_EQ(snapshot.latencyMs, 0.0);
    EXPECT_TRUE(snapshot.stages.empty());
}

TEST_F(FrameMetricsTest, GivenSteadyFrames_WhenTakingASnapshot_ThenRatesLatencyAndDropsMatch)
{
    FrameMetrics metrics;
    const FrameMetrics::Clock::time_point start = FrameMetrics::Clock::now();
    // 11 frames shown 100 ms apart (10 fps), each 20 ms after capture. The
    // source skips every other index: it captured at 20 fps.
    for (int i = 0; i <= 10; ++i) {
        const FrameMetrics::Clock::time_point captured = start + std::chrono::milliseconds(100 * i);
        metrics.add(frameAt(static_cast<std::uint64_t>(2 * i), captured), captured + std::chrono::milliseconds(20),
            DropCounts { static_cast<std::uint64_t>(i), 0 });
    }

    const MetricsSnapshot snapshot = metrics.snapshot(start + std::chrono::milliseconds(1020));
    EXPECT_NEAR(snapshot.displayFps, 10.0, 1e-9);
    EXPECT_NEAR(snapshot.captureFps, 20.0, 1e-9);
    EXPECT_NEAR(snapshot.latencyMs, 20.0, 1e-9);
    EXPECT_NEAR(snapshot.droppedBeforeProcessingPerSecond, 10.0, 1e-9);
    EXPECT_EQ(snapshot.droppedBeforeDisplayPerSecond, 0.0);
}

TEST_F(FrameMetricsTest, GivenStageTimings_WhenTakingASnapshot_ThenEachStageIsAveragedInOrder)
{
    FrameMetrics metrics;
    const FrameMetrics::Clock::time_point start = FrameMetrics::Clock::now();
    metrics.add(frameAt(0, start, { { "blur", 2.0 }, { "edges", 4.0 } }), start, {});
    metrics.add(frameAt(1, start, { { "blur", 4.0 }, { "edges", 6.0 } }), start, {});

    const MetricsSnapshot snapshot = metrics.snapshot(start);
    ASSERT_EQ(snapshot.stages.size(), 2U);
    EXPECT_EQ(snapshot.stages[0].id, "blur");
    EXPECT_NEAR(snapshot.stages[0].milliseconds, 3.0, 1e-9);
    EXPECT_EQ(snapshot.stages[1].id, "edges");
    EXPECT_NEAR(snapshot.stages[1].milliseconds, 5.0, 1e-9);
    EXPECT_NEAR(snapshot.stagesMs, 8.0, 1e-9);
}

TEST_F(FrameMetricsTest, GivenOnlyOldFrames_WhenTakingASnapshot_ThenTheyAreOutsideTheWindow)
{
    FrameMetrics metrics(std::chrono::milliseconds(1000));
    const FrameMetrics::Clock::time_point start = FrameMetrics::Clock::now();
    metrics.add(frameAt(0, start), start, {});
    metrics.add(frameAt(1, start), start + std::chrono::milliseconds(100), {});

    // Two seconds later nothing is inside the window: a stalled feed reads as zero.
    const MetricsSnapshot snapshot = metrics.snapshot(start + std::chrono::milliseconds(2100));
    EXPECT_EQ(snapshot.displayFps, 0.0);
    EXPECT_EQ(snapshot.latencyMs, 0.0);
}

TEST_F(FrameMetricsTest, GivenFrames_WhenCleared_ThenTheSnapshotIsEmpty)
{
    FrameMetrics metrics;
    const FrameMetrics::Clock::time_point start = FrameMetrics::Clock::now();
    metrics.add(frameAt(0, start), start, {});
    metrics.add(frameAt(1, start), start + std::chrono::milliseconds(50), {});
    metrics.clear();
    EXPECT_EQ(metrics.snapshot(start + std::chrono::milliseconds(50)).displayFps, 0.0);
}

TEST_F(FrameMetricsTest, GivenDetectionResults_WhenTakingASnapshot_ThenTimeRateAndAgeAreReported)
{
    FrameMetrics metrics;
    const FrameMetrics::Clock::time_point start = FrameMetrics::Clock::now();
    // Three results 500 ms apart (2 per second), each taking 400 ms and
    // arriving 450 ms after its frame was captured.
    constexpr int kResults = 3;
    constexpr int kIntervalMs = 500;
    constexpr double kDetectionMs = 400.0;
    constexpr int kDelayMs = 450;
    for (int i = 0; i < kResults; ++i) {
        const auto arrived = start + std::chrono::milliseconds(kIntervalMs * i);
        DetectionResult result;
        result.milliseconds = kDetectionMs;
        result.captured = arrived - std::chrono::milliseconds(kDelayMs);
        metrics.addDetection(result, arrived);
    }

    // Looked at the moment the last one arrived.
    const auto lastArrival = start + std::chrono::milliseconds(kIntervalMs * (kResults - 1));
    const MetricsSnapshot snapshot = metrics.snapshot(lastArrival);
    EXPECT_DOUBLE_EQ(snapshot.detectionMs, kDetectionMs);
    EXPECT_DOUBLE_EQ(snapshot.detectionsPerSecond, 1000.0 / kIntervalMs);
    EXPECT_DOUBLE_EQ(snapshot.detectionAgeMs, kDelayMs);
}

TEST_F(FrameMetricsTest, GivenDetectionResults_WhenDetectionsAreCleared_ThenTheyAreZero)
{
    FrameMetrics metrics;
    const FrameMetrics::Clock::time_point now = FrameMetrics::Clock::now();
    metrics.addDetection(DetectionResult {}, now);
    metrics.clearDetections();
    EXPECT_EQ(metrics.snapshot(now).detectionMs, 0.0);
}

} // namespace pivision::pipeline
