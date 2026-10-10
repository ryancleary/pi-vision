#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

class VideoSourceTest : public ::testing::Test {
protected:
    static constexpr int kWidth = 160;
    static constexpr int kHeight = 120;
    static constexpr int kFrameCount = 5;

    // Writes a short MJPEG AVI with OpenCV's built-in encoder, so the test needs
    // no FFmpeg or GStreamer and runs the same on the desktop, in CI and in Yocto.
    static std::string writeTestVideo()
    {
        const auto path = std::filesystem::temp_directory_path() / "pivision-test-video.avi";
        cv::VideoWriter writer(path.string(), cv::CAP_OPENCV_MJPEG,
            cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25.0, cv::Size(kWidth, kHeight));
        EXPECT_TRUE(writer.isOpened());
        for (int i = 0; i < kFrameCount; ++i)
            writer.write(cv::Mat(kHeight, kWidth, CV_8UC3, cv::Scalar(i * 40, 80, 160)));
        return path.string();
    }
};

TEST_F(VideoSourceTest, GivenAVideoFile_WhenReading_ThenFramesHaveTheFileSize)
{
    auto source = makeVideoSource(writeTestVideo());
    ASSERT_TRUE(source->open());

    Frame frame;
    ASSERT_TRUE(source->read(frame));
    EXPECT_EQ(frame.image.cols, kWidth);
    EXPECT_EQ(frame.image.rows, kHeight);
    EXPECT_EQ(frame.image.type(), CV_8UC3);
    EXPECT_DOUBLE_EQ(source->nominalFps(), 25.0);
}

TEST_F(VideoSourceTest, GivenAVideoFile_WhenReadingPastTheEnd_ThenItLoops)
{
    auto source = makeVideoSource(writeTestVideo());
    ASSERT_TRUE(source->open());

    Frame frame;
    for (int i = 0; i < kFrameCount * 2 + 1; ++i)
        ASSERT_TRUE(source->read(frame)) << "read " << i << " failed";
    EXPECT_EQ(frame.index, static_cast<std::uint64_t>(kFrameCount * 2));
}

TEST_F(VideoSourceTest, GivenAVideoFile_WhenAskingTheName_ThenItIsTheFileName)
{
    auto source = makeVideoSource("/some/dir/clip.mp4");
    EXPECT_EQ(source->name(), "clip.mp4");
}

TEST_F(VideoSourceTest, GivenAMissingFile_WhenOpening_ThenOpenFails)
{
    auto source = makeVideoSource("/nonexistent/clip.avi");
    EXPECT_FALSE(source->open());

    Frame frame;
    EXPECT_FALSE(source->read(frame));
}

TEST(SourceFromSpecTest, GivenAPathUnderDevVideo_WhenMakingASource_ThenItIsACamera)
{
    EXPECT_EQ(makeSourceFromSpec("/dev/video7")->name(), "/dev/video7");
    EXPECT_EQ(makeSourceFromSpec("/tmp/clip.avi")->name(), "clip.avi");
    EXPECT_EQ(makeSourceFromSpec("rtsp://camera.local/stream")->name(), "rtsp://camera.local/stream");
}

} // namespace pivision::capture
