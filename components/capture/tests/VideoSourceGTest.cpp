#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <pivision/capture/SourceFactory.h>

using pivision::capture::Frame;
using pivision::capture::makeSourceFromSpec;
using pivision::capture::makeVideoSource;

namespace {

constexpr int kWidth = 160;
constexpr int kHeight = 120;
constexpr int kFrameCount = 5;

// Writes a short MJPEG AVI with OpenCV's built-in encoder, so the test needs
// no FFmpeg or GStreamer and runs the same on the desktop, in CI and in Yocto.
std::string writeTestVideo()
{
    const auto path = std::filesystem::temp_directory_path() / "pivision-test-video.avi";
    cv::VideoWriter writer(path.string(), cv::CAP_OPENCV_MJPEG,
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25.0, cv::Size(kWidth, kHeight));
    EXPECT_TRUE(writer.isOpened());
    for (int i = 0; i < kFrameCount; ++i)
        writer.write(cv::Mat(kHeight, kWidth, CV_8UC3, cv::Scalar(i * 40, 80, 160)));
    return path.string();
}

} // namespace

TEST(VideoSource, ReadsFramesOfTheFileSize)
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

TEST(VideoSource, LoopsAtEndOfFile)
{
    auto source = makeVideoSource(writeTestVideo());
    ASSERT_TRUE(source->open());

    Frame frame;
    for (int i = 0; i < kFrameCount * 2 + 1; ++i)
        ASSERT_TRUE(source->read(frame)) << "read " << i << " failed";
    EXPECT_EQ(frame.index, static_cast<std::uint64_t>(kFrameCount * 2));
}

TEST(VideoSource, NameIsTheFileName)
{
    auto source = makeVideoSource("/some/dir/clip.mp4");
    EXPECT_EQ(source->name(), "clip.mp4");
}

TEST(VideoSource, OpenFailsForMissingFile)
{
    auto source = makeVideoSource("/nonexistent/clip.avi");
    EXPECT_FALSE(source->open());

    Frame frame;
    EXPECT_FALSE(source->read(frame));
}

TEST(SourceFromSpec, PathsUnderDevVideoAreCameras)
{
    EXPECT_EQ(makeSourceFromSpec("/dev/video7")->name(), "/dev/video7");
    EXPECT_EQ(makeSourceFromSpec("/tmp/clip.avi")->name(), "clip.avi");
    EXPECT_EQ(makeSourceFromSpec("rtsp://camera.local/stream")->name(), "rtsp://camera.local/stream");
}
