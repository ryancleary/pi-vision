#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>

#include "ObjectDetector.h"

namespace pivision::detection {

class ObjectDetectorTest : public ::testing::Test {
protected:
    // A typical camera frame: 640x480, the default capture size.
    static constexpr int kFrameWidth = 640;
    static constexpr int kFrameHeight = 480;

    // Letterboxing 640x480 into 416x416 scales by 416 / 640 = 0.65, giving a
    // 416x312 image with (416 - 312) / 2 = 52 rows of padding above it.
    static constexpr int kTopPaddingRows = 52;

    // Every pixel the same mid-gray value, so any pixel shows the normalization.
    static constexpr int kGray = 128;

    static cv::Mat grayFrame()
    {
        return cv::Mat(kFrameHeight, kFrameWidth, CV_8UC3, cv::Scalar(kGray, kGray, kGray));
    }

    // Reads one value from the 1 x 3 x H x W blob: image 0, `channel`, `row`, `column`.
    static float blobAt(const cv::Mat &blob, int channel, int row, int column)
    {
        return blob.at<float>(cv::Vec4i(0, channel, row, column));
    }

    // What a pixel of `value` should become in channel 0 (blue, for BGR).
    float normalizedChannel0(int value) const
    {
        const ModelConfig &config = detector_->config();
        return static_cast<float>((value - config.mean[0]) / config.standardDeviation[0]);
    }

    void SetUp() override
    {
        const std::string description = std::string(PIVISION_MODEL_DIR) + "/nanodet-plus-m-1.5x-416.json";
        std::string error;
        auto config = loadModelConfig(description, &error);
        ASSERT_TRUE(config.has_value()) << error;
        if (!std::filesystem::exists(config->modelPath))
            GTEST_SKIP() << "model not found; run scripts/fetch-assets.sh";
        inputSize_ = config->inputSize.width;
        detector_ = std::make_unique<ObjectDetector>(*config);
    }

    std::unique_ptr<ObjectDetector> detector_;
    int inputSize_ = 0; // the model's input is square: 416
};

TEST_F(ObjectDetectorTest, GivenTheModel_WhenListingOutputs_ThenThereAreTwoPerStride)
{
    // Per stride (scale), one class-score output and one box output.
    EXPECT_EQ(detector_->outputNames().size(), 2 * detector_->config().strides.size());
}

TEST_F(ObjectDetectorTest, GivenAFrame_WhenMakingTheInput_ThenItIsOneThreeChannelImageAtInputSize)
{
    const cv::Mat blob = detector_->makeInputBlob(grayFrame());

    ASSERT_EQ(blob.dims, 4);
    EXPECT_EQ(blob.size[0], 1);          // images
    EXPECT_EQ(blob.size[1], 3);          // channels
    EXPECT_EQ(blob.size[2], inputSize_); // height
    EXPECT_EQ(blob.size[3], inputSize_); // width
    EXPECT_EQ(blob.type(), CV_32F);
}

// EXPECT_FLOAT_EQ compares floats allowing for rounding: equal to within 4
// "units in the last place", i.e. the 4 nearest representable floats. Exact
// == would be fragile, since OpenCV computes in float and we in double.
// https://google.github.io/googletest/reference/assertions.html#floating-point

TEST_F(ObjectDetectorTest, GivenAGrayFrame_WhenMakingTheInput_ThenPixelsAreNormalizedPerChannel)
{
    const cv::Mat blob = detector_->makeInputBlob(grayFrame());
    const int center = inputSize_ / 2;
    EXPECT_FLOAT_EQ(blobAt(blob, 0, center, center), normalizedChannel0(kGray));
}

TEST_F(ObjectDetectorTest, GivenAWideFrame_WhenMakingTheInput_ThenPaddingFillsAboveAndBelow)
{
    const cv::Mat blob = detector_->makeInputBlob(grayFrame());
    const int center = inputSize_ / 2;

    // Just above the image: padding, which is 0 before normalization.
    EXPECT_FLOAT_EQ(blobAt(blob, 0, kTopPaddingRows - 1, center), normalizedChannel0(0));
    // First row of the image itself.
    EXPECT_FLOAT_EQ(blobAt(blob, 0, kTopPaddingRows, center), normalizedChannel0(kGray));
}

TEST_F(ObjectDetectorTest, GivenABoxInTheInput_WhenMappingBack_ThenItLandsOnTheFrame)
{
    // A box in the frame, and where letterboxing puts it in the input:
    // x and size times 0.65, y times 0.65 plus the 52 padding rows.
    const cv::Rect inFrame(100, 100, 200, 100);
    const cv::Rect inInput(65, 65 + kTopPaddingRows, 130, 65);

    EXPECT_EQ(detector_->toImageRect(inInput, cv::Size(kFrameWidth, kFrameHeight)), inFrame);
}

} // namespace pivision::detection
