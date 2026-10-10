#include <gtest/gtest.h>

#include <vector>

#include <opencv2/core.hpp>

#include "Utils.h"

namespace pivision::detection {

class UtilsTest : public ::testing::Test {
protected:
    // Any bin count works; 8 is what NanoDet uses (regMax 7: distances 0..7).
    static constexpr int kBins = 8;
    // Midpoint of 0..kBins-1, the expected value when every bin is equally likely.
    static constexpr float kMiddleBin = (kBins - 1) / 2.0F;

    // Fake outputs shaped like a model's (1 x rows x columns). The sizes are
    // arbitrary but distinct, so only the right one can match.
    static constexpr int kCells = 169;       // e.g. a 13 x 13 grid
    static constexpr int kOtherCells = 676;  // a grid size no output has
    static constexpr int kScoreColumns = 80; // e.g. one per class
    static constexpr int kBoxColumns = 32;   // e.g. 4 edges x 8 bins
    static constexpr float kScoreFill = 1.0F;
    static constexpr float kBoxFill = 2.0F;

    static std::vector<cv::Mat> fakeOutputs()
    {
        const std::vector<int> scoresShape { 1, kCells, kScoreColumns };
        const std::vector<int> boxesShape { 1, kCells, kBoxColumns };
        return { cv::Mat(scoresShape, CV_32F, cv::Scalar(kScoreFill)),
            cv::Mat(boxesShape, CV_32F, cv::Scalar(kBoxFill)) };
    }
};

// EXPECT_FLOAT_EQ: equal within 4 units in the last place; see
// https://google.github.io/googletest/reference/assertions.html#floating-point

TEST_F(UtilsTest, GivenEqualLogits_WhenTakingTheExpectedValue_ThenItIsTheMiddle)
{
    // Equal scores mean equal probabilities.
    const cv::Mat logits(1, kBins, CV_32F, cv::Scalar(0.0F));
    EXPECT_FLOAT_EQ(expectedValue(logits), kMiddleBin);
}

TEST_F(UtilsTest, GivenOneDominantBin_WhenTakingTheExpectedValue_ThenItIsThatBin)
{
    constexpr int kDominantBin = 5;
    // e^20 is about 485 million times e^0, so this bin gets almost all the probability.
    constexpr float kHighScore = 20.0F;
    cv::Mat logits(1, kBins, CV_32F, cv::Scalar(0.0F));
    logits.at<float>(0, kDominantBin) = kHighScore;
    EXPECT_FLOAT_EQ(expectedValue(logits), static_cast<float>(kDominantBin));
}

TEST_F(UtilsTest, GivenOutputsOfDifferentShapes_WhenFindingOne_ThenTheMatchingOneIsReturned)
{
    const std::vector<cv::Mat> outputs = fakeOutputs();
    const cv::Mat boxes = findOutput(outputs, kCells, kBoxColumns);

    ASSERT_FALSE(boxes.empty());
    EXPECT_EQ(boxes.rows, kCells);
    EXPECT_EQ(boxes.cols, kBoxColumns);
    EXPECT_FLOAT_EQ(boxes.at<float>(0, 0), kBoxFill); // the boxes output, not the scores
}

TEST_F(UtilsTest, GivenNoOutputOfThatShape_WhenFindingOne_ThenNothingIsReturned)
{
    EXPECT_TRUE(findOutput(fakeOutputs(), kOtherCells, kBoxColumns).empty());
}

} // namespace pivision::detection
