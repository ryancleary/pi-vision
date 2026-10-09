#include <gtest/gtest.h>

#include <chrono>

#include <pivision/pipeline/FrameMetrics.h>

namespace pivision::pipeline {

namespace {

    using Clock = FrameMetrics::Clock;
    using std::chrono::milliseconds;

    // A frame with capture index `index`, captured at `captured`, whose stages took `timings`.
    DisplayFrame frameAt(std::uint64_t index, Clock::time_point captured,
        std::vector<processing::StageTiming> timings = {})
    {
        DisplayFrame frame;
        frame.index = index;
        frame.captured = captured;
        frame.timings = std::move(timings);
        return frame;
    }

} // namespace

TEST(FrameMetricsGTest, EmptyWindowIsAllZero)
{
    const FrameMetrics metrics;
    const MetricsSnapshot snapshot = metrics.snapshot(Clock::now());
    EXPECT_EQ(snapshot.displayFps, 0.0);
    EXPECT_EQ(snapshot.captureFps, 0.0);
    EXPECT_EQ(snapshot.latencyMs, 0.0);
    EXPECT_TRUE(snapshot.stages.empty());
}

TEST(FrameMetricsGTest, RatesAndLatencyFromSteadyFrames)
{
    FrameMetrics metrics;
    const Clock::time_point start = Clock::now();
    // 11 frames shown 100 ms apart (10 fps), each 20 ms after capture. The
    // source skips every other index: it captured at 20 fps.
    for (int i = 0; i <= 10; ++i) {
        const Clock::time_point captured = start + milliseconds(100 * i);
        metrics.add(frameAt(static_cast<std::uint64_t>(2 * i), captured), captured + milliseconds(20),
            DropCounts { static_cast<std::uint64_t>(i), 0 });
    }

    const MetricsSnapshot snapshot = metrics.snapshot(start + milliseconds(1020));
    EXPECT_NEAR(snapshot.displayFps, 10.0, 1e-9);
    EXPECT_NEAR(snapshot.captureFps, 20.0, 1e-9);
    EXPECT_NEAR(snapshot.latencyMs, 20.0, 1e-9);
    EXPECT_NEAR(snapshot.droppedBeforeProcessingPerSecond, 10.0, 1e-9);
    EXPECT_EQ(snapshot.droppedBeforeDisplayPerSecond, 0.0);
}

TEST(FrameMetricsGTest, StageTimesAveragedInOrder)
{
    FrameMetrics metrics;
    const Clock::time_point start = Clock::now();
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

TEST(FrameMetricsGTest, OldFramesLeaveTheWindow)
{
    FrameMetrics metrics(milliseconds(1000));
    const Clock::time_point start = Clock::now();
    metrics.add(frameAt(0, start), start, {});
    metrics.add(frameAt(1, start), start + milliseconds(100), {});

    // Two seconds later nothing is inside the window: a stalled feed reads as zero.
    const MetricsSnapshot snapshot = metrics.snapshot(start + milliseconds(2100));
    EXPECT_EQ(snapshot.displayFps, 0.0);
    EXPECT_EQ(snapshot.latencyMs, 0.0);
}

TEST(FrameMetricsGTest, ClearForgetsFrames)
{
    FrameMetrics metrics;
    const Clock::time_point start = Clock::now();
    metrics.add(frameAt(0, start), start, {});
    metrics.add(frameAt(1, start), start + milliseconds(50), {});
    metrics.clear();
    EXPECT_EQ(metrics.snapshot(start + milliseconds(50)).displayFps, 0.0);
}

} // namespace pivision::pipeline
