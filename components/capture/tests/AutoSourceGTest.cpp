#include <gtest/gtest.h>

#include <pivision/capture/SourceFactory.h>

namespace pivision::capture {

TEST(AutoSource, FallsBackToTestPatternWhenNoCameraOpens)
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

TEST(AutoSource, ReadFailsBeforeOpenAndAfterClose)
{
    auto source = makeAutoSource(std::vector<std::string> {});
    Frame frame;
    EXPECT_FALSE(source->read(frame));

    ASSERT_TRUE(source->open());
    EXPECT_TRUE(source->read(frame));

    source->close();
    EXPECT_FALSE(source->read(frame));
    EXPECT_EQ(source->name(), "Auto");
}

} // namespace pivision::capture
