#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include <opencv2/imgcodecs.hpp>

#include "ObjectDetector.h"

namespace pivision::detection {

class ObjectDetectorTest : public ::testing::Test {
protected:
    // The model under test; its description documents every model value.
    static constexpr const char *kModelDescription = "nanodet-plus-m-1.5x-416.json";

    // A typical camera frame: the default capture size.
    static constexpr int kFrameWidth = 640;
    static constexpr int kFrameHeight = 480;

    // Every pixel the same mid-gray value, so any pixel shows the normalization.
    static constexpr int kGray = 128;

    // OpenCV's sample photo of a football player with a ball, fetched by
    // scripts/fetch-assets.sh, and the objects it shows (COCO class names).
    static constexpr const char *kSamplePhoto = "messi5.jpg";
    static inline const std::vector<std::string> kSamplePhotoObjects { "person", "sports ball" };

    // Where letterboxing puts a frame inside the model's input. The frame is
    // scaled by the smaller of the two size ratios so it fits, then centered;
    // the same formula OpenCV uses:
    //     https://github.com/opencv/opencv/blob/4.9.0/modules/dnn/src/dnn_utils.cpp#L189-L198
    // For 640x480 into 416x416: scale 0.65, a 416x312 image, 52 rows above it.
    struct Letterbox {
        double scale = 1.0;
        int topPadding = 0;
    };

    Letterbox expectedLetterbox() const
    {
        const cv::Size input = config().inputSize;
        Letterbox letterbox;
        letterbox.scale = std::min(static_cast<double>(input.width) / kFrameWidth,
            static_cast<double>(input.height) / kFrameHeight);
        const int scaledHeight = static_cast<int>(kFrameHeight * letterbox.scale);
        letterbox.topPadding = (input.height - scaledHeight) / 2;
        return letterbox;
    }

    static cv::Mat grayFrame()
    {
        return cv::Mat(kFrameHeight, kFrameWidth, CV_8UC3, cv::Scalar(kGray, kGray, kGray));
    }

    static cv::Mat samplePhoto()
    {
        return cv::imread(std::string(PIVISION_MEDIA_DIR) + "/" + kSamplePhoto);
    }

    // Reads one value from the 1 x 3 x height x width blob: image 0, `channel`, `row`, `column`.
    static float blobAt(const cv::Mat &blob, int channel, int row, int column)
    {
        return blob.at<float>(cv::Vec4i(0, channel, row, column));
    }

    const ModelConfig &config() const { return detector_->config(); }

    // What a pixel of `value` should become in channel 0 (blue, for BGR).
    float normalizedChannel0(int value) const
    {
        return static_cast<float>((value - config().mean[0]) / config().standardDeviation[0]);
    }

    // Candidates for the sample photo: prepare, run, decode.
    std::vector<Detection> samplePhotoCandidates() const
    {
        return detector_->decode(detector_->run(detector_->makeInputBlob(samplePhoto())));
    }

    // True if any candidate is of the class named `name`.
    bool found(const std::vector<Detection> &candidates, const std::string &name) const
    {
        return std::any_of(candidates.begin(), candidates.end(), [&](const Detection &candidate) {
            return config().classes.at(static_cast<size_t>(candidate.classId)) == name;
        });
    }

    void SetUp() override
    {
        std::string error;
        auto description = loadModelConfig(std::string(PIVISION_MODEL_DIR) + "/" + kModelDescription, &error);
        ASSERT_TRUE(description.has_value()) << error;
        if (!std::filesystem::exists(description->modelPath))
            GTEST_SKIP() << "model not found; run scripts/fetch-assets.sh";
        detector_ = std::make_unique<ObjectDetector>(*description);
    }

    std::unique_ptr<ObjectDetector> detector_;
};

// EXPECT_FLOAT_EQ compares floats allowing for rounding: equal to within 4
// "units in the last place", i.e. the 4 nearest representable floats. Exact
// == would be fragile, since OpenCV computes in float and we in double.
// https://google.github.io/googletest/reference/assertions.html#floating-point

TEST_F(ObjectDetectorTest, GivenTheModel_WhenListingOutputs_ThenThereAreTwoPerStride)
{
    // Per stride (scale), one class-score output and one box output.
    EXPECT_EQ(detector_->outputNames().size(), 2 * config().strides.size());
}

TEST_F(ObjectDetectorTest, GivenAFrame_WhenMakingTheInput_ThenItIsOneThreeChannelImageAtInputSize)
{
    const cv::Mat blob = detector_->makeInputBlob(grayFrame());

    ASSERT_EQ(blob.dims, 4);                              // images x channels x height x width
    EXPECT_EQ(blob.size[0], 1);                           // one image
    EXPECT_EQ(blob.size[1], 3);                           // B, G, R
    EXPECT_EQ(blob.size[2], config().inputSize.height);
    EXPECT_EQ(blob.size[3], config().inputSize.width);
    EXPECT_EQ(blob.type(), CV_32F);
}

TEST_F(ObjectDetectorTest, GivenAGrayFrame_WhenMakingTheInput_ThenPixelsAreNormalizedPerChannel)
{
    const cv::Mat blob = detector_->makeInputBlob(grayFrame());
    const cv::Point center(config().inputSize.width / 2, config().inputSize.height / 2);
    EXPECT_FLOAT_EQ(blobAt(blob, 0, center.y, center.x), normalizedChannel0(kGray));
}

TEST_F(ObjectDetectorTest, GivenAWideFrame_WhenMakingTheInput_ThenPaddingFillsAboveAndBelow)
{
    const cv::Mat blob = detector_->makeInputBlob(grayFrame());
    const int column = config().inputSize.width / 2;
    const int firstImageRow = expectedLetterbox().topPadding;

    // Just above the image: padding, which is 0 before normalization.
    EXPECT_FLOAT_EQ(blobAt(blob, 0, firstImageRow - 1, column), normalizedChannel0(0));
    // First row of the image itself.
    EXPECT_FLOAT_EQ(blobAt(blob, 0, firstImageRow, column), normalizedChannel0(kGray));
}

TEST_F(ObjectDetectorTest, GivenABoxInTheInput_WhenMappingBack_ThenItLandsOnTheFrame)
{
    // A box in the frame, and where letterboxing puts it in the input: scaled,
    // and moved down by the top padding.
    const cv::Rect inFrame(100, 100, 200, 100);
    const Letterbox letterbox = expectedLetterbox();
    const cv::Rect inInput(static_cast<int>(inFrame.x * letterbox.scale),
        static_cast<int>(inFrame.y * letterbox.scale) + letterbox.topPadding,
        static_cast<int>(inFrame.width * letterbox.scale),
        static_cast<int>(inFrame.height * letterbox.scale));

    EXPECT_EQ(detector_->toImageRect(inInput, cv::Size(kFrameWidth, kFrameHeight)), inFrame);
}

TEST_F(ObjectDetectorTest, GivenTheSamplePhoto_WhenDecoding_ThenItsObjectsAreFound)
{
    if (samplePhoto().empty())
        GTEST_SKIP() << "sample photo not found; run scripts/fetch-assets.sh";

    const std::vector<Detection> candidates = samplePhotoCandidates();
    for (const std::string &object : kSamplePhotoObjects)
        EXPECT_TRUE(found(candidates, object)) << object;
}

TEST_F(ObjectDetectorTest, GivenTheSamplePhoto_WhenDecoding_ThenCandidatesAreConfidentAndInsideTheInput)
{
    if (samplePhoto().empty())
        GTEST_SKIP() << "sample photo not found; run scripts/fetch-assets.sh";

    const cv::Rect inputArea(cv::Point(0, 0), config().inputSize);
    for (const Detection &candidate : samplePhotoCandidates()) {
        EXPECT_GE(candidate.score, config().scoreThreshold);
        EXPECT_EQ(candidate.box & inputArea, candidate.box); // & is the overlap of two rects
    }
}

TEST_F(ObjectDetectorTest, GivenAPlainGrayFrame_WhenDecoding_ThenNothingIsFound)
{
    EXPECT_TRUE(detector_->decode(detector_->run(detector_->makeInputBlob(grayFrame()))).empty());
}

} // namespace pivision::detection
