#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include <pivision/capture/SourceFactory.h>

using pivision::capture::Frame;
using pivision::capture::makeTestPatternSource;
using pivision::capture::TestPatternConfig;

namespace {

TestPatternConfig smallConfig()
{
    TestPatternConfig config;
    config.width = 320;
    config.height = 240;
    config.fps = 30.0;
    return config;
}

} // namespace

TEST(TestPatternSource, ProducesFramesOfConfiguredSizeAndType)
{
    auto source = makeTestPatternSource(smallConfig());
    ASSERT_TRUE(source->open());

    Frame frame;
    ASSERT_TRUE(source->read(frame));
    EXPECT_EQ(frame.image.cols, 320);
    EXPECT_EQ(frame.image.rows, 240);
    EXPECT_EQ(frame.image.type(), CV_8UC3);
}

TEST(TestPatternSource, IndexStartsAtZeroAndIncrements)
{
    auto source = makeTestPatternSource(smallConfig());
    ASSERT_TRUE(source->open());

    Frame frame;
    for (std::uint64_t expected = 0; expected < 3; ++expected) {
        ASSERT_TRUE(source->read(frame));
        EXPECT_EQ(frame.index, expected);
    }
}

TEST(TestPatternSource, ConsecutiveFramesDiffer)
{
    auto source = makeTestPatternSource(smallConfig());
    ASSERT_TRUE(source->open());

    Frame first;
    Frame second;
    ASSERT_TRUE(source->read(first));
    ASSERT_TRUE(source->read(second));
    EXPECT_GT(cv::norm(first.image, second.image, cv::NORM_L1), 0.0);
}

TEST(TestPatternSource, IsDeterministic)
{
    auto a = makeTestPatternSource(smallConfig());
    auto b = makeTestPatternSource(smallConfig());
    ASSERT_TRUE(a->open());
    ASSERT_TRUE(b->open());

    Frame frameA;
    Frame frameB;
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(a->read(frameA));
        ASSERT_TRUE(b->read(frameB));
    }
    EXPECT_EQ(cv::norm(frameA.image, frameB.image, cv::NORM_L1), 0.0);
}

TEST(TestPatternSource, ReadFailsBeforeOpen)
{
    auto source = makeTestPatternSource(smallConfig());
    Frame frame;
    EXPECT_FALSE(source->read(frame));
}

TEST(TestPatternSource, OpenFailsForInvalidSize)
{
    TestPatternConfig config = smallConfig();
    config.width = 0;
    EXPECT_FALSE(makeTestPatternSource(config)->open());
}

TEST(TestPatternSource, OpenFailsForInvalidFrameRate)
{
    TestPatternConfig config = smallConfig();
    config.fps = 0.0;
    EXPECT_FALSE(makeTestPatternSource(config)->open());
}

