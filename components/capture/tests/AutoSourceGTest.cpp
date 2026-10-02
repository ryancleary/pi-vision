#include <gtest/gtest.h>

#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

TEST(AutoSourceTest, GivenNoCameraOpens_WhenOpening_ThenItFallsBackToTheTestPattern)
{
    TestPatternConfig fallback;
    fallback.width = 320;
    fallback.height = 240;
    auto source = makeAutoSource({ "/nonexistent/video0", "/nonexistent/video1" }, {}, fallback);

    EXPECT_EQ(source->name(), "Auto");
    ASSERT_TRUE(source->open());
    EXPECT_EQ(source->name(), "Test pattern");

    Frame frame;
    ASSERT_TRUE(source->read(frame));
    EXPECT_EQ(frame.image.cols, 320);
    EXPECT_EQ(frame.image.rows, 240);
}

TEST(AutoSourceTest, GivenAClosedSource_WhenReading_ThenReadFailsButTheChoiceIsKept)
{
    auto source = makeAutoSource(std::vector<std::string> {});
    Frame frame;
    EXPECT_FALSE(source->read(frame));

    ASSERT_TRUE(source->open());
    EXPECT_TRUE(source->read(frame));

    // Closed, as when its camera drops out: it keeps the name of what it chose,
    // so the UI still says which source it is waiting for.
    source->close();
    EXPECT_FALSE(source->read(frame));
    EXPECT_EQ(source->name(), "Test pattern");
}

TEST(AutoSourceTest, GivenAClosedSource_WhenReopened_ThenTheSameChoiceReads)
{
    auto source = makeAutoSource(std::vector<std::string> {});
    ASSERT_TRUE(source->open());
    source->close();

    ASSERT_TRUE(source->reopen());
    Frame frame;
    EXPECT_TRUE(source->read(frame));
    EXPECT_EQ(source->name(), "Test pattern");
}

} // namespace pivision::capture
