#include <gtest/gtest.h>

#include <initializer_list>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <pivision/processing/StageFactory.h>

namespace pivision::processing {

namespace {

// A dim, low-contrast gradient with a bright square: something every stage
// visibly changes.
cv::Mat sampleImage()
{
    cv::Mat image(120, 160, CV_8UC3);
    for (int x = 0; x < image.cols; ++x)
        image.col(x).setTo(cv::Scalar(40 + x / 8, 50 + x / 10, 60));
    cv::rectangle(image, cv::Rect(60, 40, 40, 40), cv::Scalar(200, 220, 240), cv::FILLED);
    return image;
}

} // namespace

TEST(BuiltinStages, EveryKnownIdMakesAStageWithThatId)
{
    for (const std::string &id : knownStageIds()) {
        auto stage = makeStage(id);
        ASSERT_NE(stage, nullptr) << id;
        EXPECT_EQ(stage->id(), id);
    }
    EXPECT_EQ(makeStage("sharpen"), nullptr);
}

TEST(BuiltinStages, EveryStageAcceptsColorAndGrayAndKeepsTheSize)
{
    const cv::Mat color = sampleImage();
    cv::Mat gray;
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    for (const std::string &id : knownStageIds()) {
        auto stage = makeStage(id);
        for (const cv::Mat *input : std::initializer_list<const cv::Mat *> { &color, &gray }) {
            cv::Mat out;
            stage->process(*input, out);
            EXPECT_EQ(out.size(), input->size()) << id;
            EXPECT_EQ(out.depth(), CV_8U) << id;
        }
    }
}

TEST(BuiltinStages, GrayscaleAndEdgesProduceOneChannel)
{
    const cv::Mat color = sampleImage();
    for (const char *id : { "grayscale", "edges" }) {
        cv::Mat out;
        makeStage(id)->process(color, out);
        EXPECT_EQ(out.channels(), 1) << id;
    }
}

TEST(BuiltinStages, ClaheAndBlurKeepColorAndChangeTheImage)
{
    const cv::Mat color = sampleImage();
    for (const char *id : { "clahe", "blur" }) {
        cv::Mat out;
        makeStage(id)->process(color, out);
        EXPECT_EQ(out.channels(), 3) << id;
        EXPECT_GT(cv::norm(color, out, cv::NORM_L1), 0.0) << id;
    }
}

TEST(BuiltinStages, EdgesFindTheSquareOutline)
{
    cv::Mat out;
    makeStage("edges")->process(sampleImage(), out);
    // The square's border is an edge; its flat interior is not.
    EXPECT_GT(cv::countNonZero(out.colRange(55, 65)), 0);
    EXPECT_EQ(cv::countNonZero(out(cv::Rect(70, 50, 20, 20))), 0);
}

TEST(BuiltinStages, ParametersAreClampedAndUnknownNamesRejected)
{
    auto blur = makeStage("blur");
    EXPECT_TRUE(blur->setParameter("size", 6));
    EXPECT_EQ(blur->parameter("size"), 7.0); // even sizes round up to odd
    EXPECT_TRUE(blur->setParameter("size", 100));
    EXPECT_EQ(blur->parameter("size"), 31.0);
    EXPECT_FALSE(blur->setParameter("radius", 3));

    auto edges = makeStage("edges");
    EXPECT_TRUE(edges->setParameter("high", 999));
    EXPECT_EQ(edges->parameter("high"), 255.0);

    auto clahe = makeStage("clahe");
    EXPECT_TRUE(clahe->setParameter("clipLimit", 4.0));
    EXPECT_EQ(clahe->parameter("clipLimit"), 4.0);

    EXPECT_FALSE(makeStage("grayscale")->setParameter("anything", 1));
}

} // namespace pivision::processing
