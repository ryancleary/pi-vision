#include <gtest/gtest.h>

#include <QImage>
#include <QSignalSpy>

#include <pivision/capture/SourceFactory.h>
#include <pivision/pipeline/Pipeline.h>
#include <pivision/testsupport/QtEventLoopFixture.h>

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

