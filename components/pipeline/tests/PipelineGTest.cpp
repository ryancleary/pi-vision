#include <gtest/gtest.h>

#include <filesystem>
#include <functional>

#include <QElapsedTimer>
#include <QImage>
#include <QSignalSpy>
#include <QTest>

#include <pivision/capture/SourceFactory.h>
#include <pivision/pipeline/Pipeline.h>
#include <pivision/pipeline/StageConfig.h>
#include <pivision/testsupport/QtEventLoopFixture.h>

namespace pivision::pipeline {

// Tests here need a Qt event loop: signals cross threads.
class PipelineTest : public testsupport::QtEventLoopFixture {
protected:
    static capture::TestPatternConfig patternOfWidth(int width)
    {
        capture::TestPatternConfig config;
        config.width = width;
        config.height = width * 3 / 4;
        config.fps = 60.0;
        return config;
    }

    // Waits until a frame matching `accept` is the latest one.
    static bool waitForFrame(Pipeline &pipeline, const std::function<bool(const DisplayFrame &)> &accept,
        int timeoutMs = 3000)
    {
        QSignalSpy available(&pipeline, &Pipeline::frameAvailable);
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < timeoutMs) {
            if (auto frame = pipeline.latestFrame(); frame && accept(*frame))
                return true;
            available.wait(100);
        }
        return false;
    }

    static bool waitForRawWidth(Pipeline &pipeline, int width)
    {
        return waitForFrame(pipeline, [width](const DisplayFrame &f) { return f.raw.width() == width; });
    }

    static StageConfig stage(const char *id, bool enabled)
    {
        StageConfig config;
        config.id = QString::fromLatin1(id);
        config.enabled = enabled;
        return config;
    }

    // The detection model (scripts/fetch-assets.sh fetches the .onnx next to it).
    static QString modelDescription()
    {
        return QStringLiteral(PIVISION_MODEL_DIR "/nanodet-plus-m-1.5x-416.json");
    }

    static bool modelFetched()
    {
        return std::filesystem::exists(PIVISION_MODEL_DIR "/nanodet-plus-m-1.5x-416.onnx");
    }

    // Detection is slow (hundreds of ms on a Pi, the first run also loads the
    // model), so these waits are longer than for frames.
    static constexpr int kDetectionTimeoutMs = 15000;

    // Waits until a detection result matching `accept` is the latest one.
    static bool waitForDetection(Pipeline &pipeline,
        const std::function<bool(const DetectionResult &)> &accept = [](const DetectionResult &) { return true; })
    {
        QSignalSpy available(&pipeline, &Pipeline::detectionAvailable);
        QElapsedTimer timer;
        timer.start();
        while (timer.elapsed() < kDetectionTimeoutMs) {
            if (auto result = pipeline.latestDetection(); result && accept(*result))
                return true;
            available.wait(100);
        }
        return false;
    }

    // Waits until the detector reaches `state`.
    static bool waitForDetectorState(Pipeline &pipeline, DetectorState state)
    {
        QSignalSpy changed(&pipeline, &Pipeline::detectorStateChanged);
        QElapsedTimer timer;
        timer.start();
        while (pipeline.detectorState() != state && timer.elapsed() < kDetectionTimeoutMs)
            changed.wait(100);
        return pipeline.detectorState() == state;
    }
};

TEST_F(PipelineTest, GivenNoStages_WhenRunning_ThenRawAndProcessedComeFromTheSameCapture)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.start();
    ASSERT_TRUE(waitForRawWidth(pipeline, 320));

    const auto frame = pipeline.latestFrame();
    EXPECT_EQ(frame->raw.height(), 240);
    EXPECT_EQ(frame->raw.format(), QImage::Format_BGR888);
    // No stages enabled and no scaling: processed is the raw image itself.
    EXPECT_EQ(frame->processed, frame->raw);
    EXPECT_TRUE(frame->timings.empty());
}

TEST_F(PipelineTest, GivenALargeSource_WhenRunning_ThenProcessedFramesShrinkToTheProcessSize)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(640)));
    pipeline.setProcessSize(QSize(320, 240));
    pipeline.start();
    ASSERT_TRUE(waitForRawWidth(pipeline, 640));

    const auto frame = pipeline.latestFrame();
    EXPECT_EQ(frame->processed.size(), QSize(320, 240));
    EXPECT_EQ(frame->raw.size(), QSize(640, 480)); // raw stays full size
}

TEST_F(PipelineTest, GivenASmallSource_WhenRunning_ThenProcessedFramesAreNotEnlarged)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(160)));
    pipeline.setProcessSize(QSize(320, 240));
    pipeline.start();
    ASSERT_TRUE(waitForRawWidth(pipeline, 160));
    EXPECT_EQ(pipeline.latestFrame()->processed.size(), QSize(160, 120));
}

TEST_F(PipelineTest, GivenEnabledStages_WhenRunning_ThenTheyShapeTheProcessedFrame)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.setStages({ stage("blur", true), stage("edges", true) });
    pipeline.start();

    ASSERT_TRUE(waitForFrame(pipeline, [](const DisplayFrame &f) {
        return f.processed.format() == QImage::Format_Grayscale8;
    }));
    const auto timings = pipeline.latestFrame()->timings;
    ASSERT_EQ(timings.size(), 2u);
    EXPECT_EQ(timings[0].id, "blur");
    EXPECT_EQ(timings[1].id, "edges");
}

TEST_F(PipelineTest, GivenARunningPipeline_WhenAStageIsToggled_ThenTheOutputFollows)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.setStages({ stage("grayscale", false) });
    pipeline.start();
    ASSERT_TRUE(waitForFrame(
        pipeline, [](const DisplayFrame &f) { return f.processed.format() == QImage::Format_BGR888; }));

    pipeline.setStageEnabled(QStringLiteral("grayscale"), true);
    EXPECT_TRUE(waitForFrame(pipeline,
        [](const DisplayFrame &f) { return f.processed.format() == QImage::Format_Grayscale8; }));

    pipeline.setStageEnabled(QStringLiteral("grayscale"), false);
    EXPECT_TRUE(waitForFrame(
        pipeline, [](const DisplayFrame &f) { return f.processed.format() == QImage::Format_BGR888; }));
}

TEST_F(PipelineTest, GivenARunningPipeline_WhenTheSourceIsSwitched_ThenFramesComeFromTheNewSource)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.start();
    ASSERT_TRUE(waitForRawWidth(pipeline, 320));

    QSignalSpy cleared(&pipeline, &Pipeline::cleared);
    pipeline.setSource(capture::makeTestPatternSource(patternOfWidth(160)));
    EXPECT_EQ(cleared.count(), 1);
    EXPECT_FALSE(pipeline.latestFrame().has_value()); // the old feed is cut at once
    EXPECT_TRUE(waitForRawWidth(pipeline, 160));
    EXPECT_TRUE(pipeline.errorString().isEmpty());
}

TEST_F(PipelineTest, GivenARunningPipeline_WhenSwitchedToAFailingSource_ThenAnErrorIsReportedAndFramesStop)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.start();
    ASSERT_TRUE(waitForRawWidth(pipeline, 320));

    QSignalSpy failed(&pipeline, &Pipeline::failed);
    pipeline.setSource(capture::makeCameraSource("/nonexistent/video0"));
    ASSERT_TRUE(failed.wait(2000));
    EXPECT_FALSE(pipeline.errorString().isEmpty());

    QSignalSpy available(&pipeline, &Pipeline::frameAvailable);
    QTest::qWait(250);
    EXPECT_EQ(available.count(), 0);
    EXPECT_FALSE(pipeline.latestFrame().has_value());
}

TEST_F(PipelineTest, GivenAFailedSource_WhenAWorkingSourceIsSet_ThenFramesResume)
{
    Pipeline pipeline(capture::makeCameraSource("/nonexistent/video0"));
    QSignalSpy failed(&pipeline, &Pipeline::failed);
    pipeline.start();
    ASSERT_TRUE(failed.wait(2000));

    pipeline.setSource(capture::makeTestPatternSource(patternOfWidth(320)));
    EXPECT_TRUE(waitForRawWidth(pipeline, 320));
    EXPECT_TRUE(pipeline.errorString().isEmpty());
}

TEST_F(PipelineTest, GivenASourceSwitch_WhenTheNewSourceOpens_ThenTheSourceNameFollows)
{
    Pipeline pipeline(capture::makeAutoSource(std::vector<std::string> {}, {}, patternOfWidth(320)));
    EXPECT_EQ(pipeline.sourceName(), QStringLiteral("Auto"));

    QSignalSpy nameChanged(&pipeline, &Pipeline::sourceNameChanged);
    pipeline.start();
    ASSERT_TRUE(nameChanged.wait(2000));
    EXPECT_EQ(pipeline.sourceName(), QStringLiteral("Test pattern"));
}

TEST_F(PipelineTest, GivenASourceSetBeforeStart_WhenStarted_ThenThatSourceRuns)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.setSource(capture::makeTestPatternSource(patternOfWidth(160)));
    pipeline.start();
    EXPECT_TRUE(waitForRawWidth(pipeline, 160));
}

TEST_F(PipelineTest, GivenAPipeline_WhenStoppedWithoutStartOrTwice_ThenNothingBreaks)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.stop();
    pipeline.start();
    pipeline.stop();
    pipeline.stop();
    SUCCEED();
}

TEST_F(PipelineTest, GivenArrivingFrames_WhenTheSourceIsSwitched_ThenMetricsReset)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(160)));
    pipeline.start();
    // A rate needs at least two frames in the window; wait for a few.
    ASSERT_TRUE(waitForFrame(pipeline, [](const DisplayFrame &f) { return f.index >= 5; }));
    EXPECT_GT(pipeline.metrics().displayFps, 0.0);

    pipeline.setSource(capture::makeTestPatternSource(patternOfWidth(320)));
    EXPECT_EQ(pipeline.metrics().displayFps, 0.0);
}

TEST_F(PipelineTest, GivenDetectionOff_WhenRunning_ThenNoDetectionsArrive)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.setModelDescription(modelDescription());
    pipeline.start();
    ASSERT_TRUE(waitForRawWidth(pipeline, 320));
    EXPECT_FALSE(pipeline.latestDetection().has_value());
    EXPECT_EQ(pipeline.detectorState(), DetectorState::Off); // the model isn't even loaded
}

TEST_F(PipelineTest, GivenRawInput_WhenDetecting_ThenResultsAreOnRawSizedImages)
{
    if (!modelFetched())
        GTEST_SKIP() << "model not found; run scripts/fetch-assets.sh";
    constexpr int kRawWidth = 640;
    constexpr int kProcessWidth = 320; // smaller, so the two inputs are told apart by size
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(kRawWidth)));
    pipeline.setProcessSize(QSize(kProcessWidth, kProcessWidth * 3 / 4));
    pipeline.setModelDescription(modelDescription());
    pipeline.setDetectionEnabled(true);
    pipeline.start();

    ASSERT_TRUE(waitForDetection(pipeline));
    const auto result = pipeline.latestDetection();
    EXPECT_EQ(result->input, DetectionInput::Raw);
    EXPECT_EQ(result->image.width(), kRawWidth);
    EXPECT_GT(result->milliseconds, 0.0);
    EXPECT_EQ(pipeline.detectorState(), DetectorState::Ready);
}

TEST_F(PipelineTest, GivenProcessedInput_WhenDetecting_ThenResultsAreOnProcessedImages)
{
    if (!modelFetched())
        GTEST_SKIP() << "model not found; run scripts/fetch-assets.sh";
    constexpr int kRawWidth = 640;
    constexpr int kProcessWidth = 320;
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(kRawWidth)));
    pipeline.setProcessSize(QSize(kProcessWidth, kProcessWidth * 3 / 4));
    // Edges leaves one gray channel; the detector still needs three.
    pipeline.setStages({ stage("edges", true) });
    pipeline.setModelDescription(modelDescription());
    pipeline.setDetectionInput(DetectionInput::Processed);
    pipeline.setDetectionEnabled(true);
    pipeline.start();

    ASSERT_TRUE(waitForDetection(pipeline));
    EXPECT_EQ(pipeline.latestDetection()->input, DetectionInput::Processed);
    EXPECT_EQ(pipeline.latestDetection()->image.width(), kProcessWidth);
}

TEST_F(PipelineTest, GivenDetectionRunning_WhenSwitchedOff_ThenTheLatestDetectionIsCleared)
{
    if (!modelFetched())
        GTEST_SKIP() << "model not found; run scripts/fetch-assets.sh";
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.setModelDescription(modelDescription());
    pipeline.setDetectionEnabled(true);
    pipeline.start();
    ASSERT_TRUE(waitForDetection(pipeline));

    pipeline.setDetectionEnabled(false);
    EXPECT_FALSE(pipeline.latestDetection().has_value());
    EXPECT_EQ(pipeline.metrics().detectionsPerSecond, 0.0);
}

TEST_F(PipelineTest, GivenAMissingModel_WhenDetectionIsEnabled_ThenTheStateIsFailedWithAReason)
{
    Pipeline pipeline(capture::makeTestPatternSource(patternOfWidth(320)));
    pipeline.setModelDescription(QStringLiteral("/nonexistent/model.json"));
    pipeline.setDetectionEnabled(true);
    pipeline.start();

    ASSERT_TRUE(waitForDetectorState(pipeline, DetectorState::Failed));
    EXPECT_FALSE(pipeline.detectorError().isEmpty());
    EXPECT_FALSE(pipeline.latestDetection().has_value());
}

} // namespace pivision::pipeline
