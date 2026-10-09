#include <gtest/gtest.h>

#include <QElapsedTimer>
#include <QImage>
#include <QSignalSpy>
#include <QTest>

#include <pivision/capture/SourceFactory.h>
#include <pivision/pipeline/Pipeline.h>
#include <pivision/testsupport/QtEventLoopFixture.h>

using pivision::capture::makeAutoSource;
using pivision::capture::makeCameraSource;
using pivision::capture::makeTestPatternSource;
using pivision::capture::TestPatternConfig;
using pivision::pipeline::Pipeline;

using PipelineTest = pivision::testsupport::QtEventLoopFixture;

namespace {

TestPatternConfig smallConfig()
{
    TestPatternConfig config;
    config.width = 320;
    config.height = 240;
    config.fps = 60.0;
    return config;
}

TestPatternConfig patternOfWidth(int width)
{
    TestPatternConfig config = smallConfig();
    config.width = width;
    config.height = width * 3 / 4;
    return config;
}

// Waits until a frame of `width` arrives, skipping any older frames.
bool waitForFrameOfWidth(Pipeline &pipeline, int width, int timeoutMs = 2000)
{
    QSignalSpy available(&pipeline, &Pipeline::frameAvailable);
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        if (auto frame = pipeline.takeLatestFrame(); frame && frame->image.width() == width)
            return true;
        available.wait(100);
    }
    return false;
}

} // namespace

TEST_F(PipelineTest, DeliversFramesFromCaptureThread)
{
    Pipeline pipeline(makeTestPatternSource(smallConfig()));
    QSignalSpy available(&pipeline, &Pipeline::frameAvailable);

    pipeline.start();
    ASSERT_TRUE(available.wait(2000));

    auto frame = pipeline.takeLatestFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(frame->image.width(), 320);
    EXPECT_EQ(frame->image.height(), 240);
    EXPECT_EQ(frame->image.format(), QImage::Format_BGR888);
}

TEST_F(PipelineTest, ReportsFailureWhenSourceCannotOpen)
{
    TestPatternConfig config = smallConfig();
    config.width = 0;
    Pipeline pipeline(makeTestPatternSource(config));
    QSignalSpy failed(&pipeline, &Pipeline::failed);

    pipeline.start();
    ASSERT_TRUE(failed.wait(2000));
    EXPECT_FALSE(pipeline.takeLatestFrame().has_value());
}

TEST_F(PipelineTest, StopIsSafeWithoutStartAndWhenRepeated)
{
    Pipeline pipeline(makeTestPatternSource(smallConfig()));
    pipeline.stop();
    pipeline.start();
    pipeline.stop();
    pipeline.stop();
    SUCCEED();
}

TEST_F(PipelineTest, SwitchingSourceDeliversFramesFromTheNewSource)
{
    Pipeline pipeline(makeTestPatternSource(patternOfWidth(320)));
    pipeline.start();
    ASSERT_TRUE(waitForFrameOfWidth(pipeline, 320));

    QSignalSpy cleared(&pipeline, &Pipeline::cleared);
    pipeline.setSource(makeTestPatternSource(patternOfWidth(160)));
    EXPECT_EQ(cleared.count(), 1);
    EXPECT_TRUE(waitForFrameOfWidth(pipeline, 160));
    EXPECT_TRUE(pipeline.errorString().isEmpty());
}

TEST_F(PipelineTest, SwitchingToSourceThatFailsReportsErrorAndStopsFrames)
{
    Pipeline pipeline(makeTestPatternSource(smallConfig()));
    pipeline.start();
    ASSERT_TRUE(waitForFrameOfWidth(pipeline, 320));

    QSignalSpy failed(&pipeline, &Pipeline::failed);
    pipeline.setSource(makeCameraSource("/nonexistent/video0"));
    ASSERT_TRUE(failed.wait(2000));
    EXPECT_FALSE(pipeline.errorString().isEmpty());

    // The old feed is cut: nothing new arrives after the failure.
    pipeline.takeLatestFrame();
    QSignalSpy available(&pipeline, &Pipeline::frameAvailable);
    QTest::qWait(200);
    EXPECT_EQ(available.count(), 0);
    EXPECT_FALSE(pipeline.takeLatestFrame().has_value());
}

TEST_F(PipelineTest, RecoversWhenAWorkingSourceIsSetAfterAFailure)
{
    Pipeline pipeline(makeCameraSource("/nonexistent/video0"));
    QSignalSpy failed(&pipeline, &Pipeline::failed);
    pipeline.start();
    ASSERT_TRUE(failed.wait(2000));

    pipeline.setSource(makeTestPatternSource(smallConfig()));
    EXPECT_TRUE(waitForFrameOfWidth(pipeline, 320));
    EXPECT_TRUE(pipeline.errorString().isEmpty());
}

TEST_F(PipelineTest, SourceNameFollowsTheSourceThatOpened)
{
    // Auto with no cameras falls back to the test pattern; the name only
    // becomes known once the source opens on the capture thread.
    Pipeline pipeline(makeAutoSource(std::vector<std::string> {}, {}, smallConfig()));
    EXPECT_EQ(pipeline.sourceName(), QStringLiteral("Auto"));

    QSignalSpy nameChanged(&pipeline, &Pipeline::sourceNameChanged);
    pipeline.start();
    ASSERT_TRUE(nameChanged.wait(2000));
    EXPECT_EQ(pipeline.sourceName(), QStringLiteral("Test pattern"));
}

TEST_F(PipelineTest, SourceSetBeforeStartIsTheOneThatRuns)
{
    Pipeline pipeline(makeTestPatternSource(patternOfWidth(320)));
    pipeline.setSource(makeTestPatternSource(patternOfWidth(160)));
    pipeline.start();
    EXPECT_TRUE(waitForFrameOfWidth(pipeline, 160));
}
