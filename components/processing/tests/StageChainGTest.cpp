#include <gtest/gtest.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <pivision/processing/StageChain.h>
#include <pivision/processing/StageFactory.h>

namespace pivision::processing {

class StageChainTest : public ::testing::Test {
protected:
    static cv::Mat sampleImage()
    {
        cv::Mat image(120, 160, CV_8UC3, cv::Scalar(30, 60, 90));
        cv::circle(image, cv::Point(80, 60), 30, cv::Scalar(220, 220, 220), cv::FILLED);
        return image;
    }

    static StageChain fullChain(bool enabled)
    {
        StageChain chain;
        for (const char *id : { "clahe", "grayscale", "blur", "edges" })
            chain.add(makeStage(id), enabled);
        return chain;
    }
};

TEST_F(StageChainTest, GivenStagesAdded_WhenListingTheOrder_ThenItIsTheOrderAdded)
{
    StageChain chain = fullChain(false);
    EXPECT_EQ(chain.order(), (std::vector<std::string> { "clahe", "grayscale", "blur", "edges" }));
}

TEST_F(StageChainTest, GivenNoEnabledStages_WhenRunning_ThenOutputEqualsInput)
{
    StageChain chain = fullChain(false);
    EXPECT_FALSE(chain.anyEnabled());

    const cv::Mat in = sampleImage();
    cv::Mat out;
    std::vector<StageTiming> timings;
    chain.run(in, out, &timings);
    EXPECT_EQ(cv::norm(in, out, cv::NORM_L1), 0.0);
    EXPECT_TRUE(timings.empty());
}

TEST_F(StageChainTest, GivenSomeEnabledStages_WhenRunning_ThenOnlyThoseRunInOrderAndAreTimed)
{
    StageChain chain = fullChain(false);
    ASSERT_TRUE(chain.setEnabled("blur", true));
    ASSERT_TRUE(chain.setEnabled("grayscale", true));

    cv::Mat out;
    std::vector<StageTiming> timings;
    chain.run(sampleImage(), out, &timings);

    ASSERT_EQ(timings.size(), 2u);
    EXPECT_EQ(timings[0].id, "grayscale"); // chain order, not the order enabled
    EXPECT_EQ(timings[1].id, "blur");
    EXPECT_GE(timings[0].milliseconds, 0.0);
    EXPECT_EQ(out.channels(), 1);
}

TEST_F(StageChainTest, GivenAnOutput_WhenTheChainRunsAgain_ThenTheEarlierOutputIsUnchanged)
{
    StageChain chain = fullChain(false);
    chain.setEnabled("blur", true);

    cv::Mat first;
    chain.run(sampleImage(), first);
    const cv::Mat saved = first.clone();

    cv::Mat second;
    chain.run(cv::Mat(120, 160, CV_8UC3, cv::Scalar(0, 0, 0)), second);
    EXPECT_EQ(cv::norm(first, saved, cv::NORM_L1), 0.0);
}

TEST_F(StageChainTest, GivenUnknownIds_WhenSettingThem_ThenFailureIsReported)
{
    StageChain chain = fullChain(true);
    EXPECT_FALSE(chain.setEnabled("sharpen", true));
    EXPECT_FALSE(chain.setParameter("sharpen", "amount", 1.0));
    EXPECT_FALSE(chain.setParameter("blur", "radius", 1.0));
    EXPECT_TRUE(chain.setParameter("blur", "size", 9.0));
}

TEST_F(StageChainTest, GivenEveryStageEnabled_WhenRunning_ThenTheOutputIsAnEdgeMap)
{
    StageChain chain = fullChain(true);
    cv::Mat out;
    chain.run(sampleImage(), out);
    EXPECT_EQ(out.channels(), 1);
    EXPECT_GT(cv::countNonZero(out), 0);
}

} // namespace pivision::processing
