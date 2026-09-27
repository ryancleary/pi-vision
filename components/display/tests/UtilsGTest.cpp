#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include <QJsonArray>
#include <QJsonObject>

#include "Utils.h"

namespace pivision::display {

class UtilsTest : public ::testing::Test {
protected:
    // A made-up model with two classes and a result holding one box of each,
    // plus one class id the model has no name for.
    static constexpr int kPersonId = 0;
    static constexpr int kBallId = 1;
    static constexpr int kUnnamedId = 7;
    static constexpr int kImageWidth = 640;
    static constexpr int kImageHeight = 480;
    static constexpr int kModelInput = 416;
    static constexpr std::uint64_t kFrameIndex = 42;
    static constexpr double kMilliseconds = 86.5;
    static constexpr float kPersonScore = 0.75F;
    static constexpr float kBallScore = 0.5F;
    // Any box inside the image.
    static constexpr int kBoxX = 10;
    static constexpr int kBoxY = 20;
    static constexpr int kBoxWidth = 30;
    static constexpr int kBoxHeight = 40;

    static pipeline::DetectionResult makeResult()
    {
        pipeline::DetectionResult result;
        result.image = QImage(kImageWidth, kImageHeight, QImage::Format_RGB888);
        result.input = pipeline::DetectionInput::Processed;
        result.frameIndex = kFrameIndex;
        result.milliseconds = kMilliseconds;
        result.modelInputSize = QSize(kModelInput, kModelInput);
        result.classNames = std::make_shared<const std::vector<std::string>>(
            std::vector<std::string> { "person", "sports ball" });
        const cv::Rect box(kBoxX, kBoxY, kBoxWidth, kBoxHeight);
        result.detections = {
            { box, kPersonId, kPersonScore },
            { box, kBallId, kBallScore },
        };
        return result;
    }
};

TEST_F(UtilsTest, GivenANamedClass_WhenLabelling_ThenTheModelsNameIsUsed)
{
    EXPECT_EQ(classLabel(makeResult(), kBallId), QStringLiteral("sports ball"));
}

TEST_F(UtilsTest, GivenAClassWithoutAName_WhenLabelling_ThenTheNumberIsUsed)
{
    EXPECT_EQ(classLabel(makeResult(), kUnnamedId), QString::number(kUnnamedId));
}

TEST_F(UtilsTest, GivenAResult_WhenWritingJson_ThenItDescribesTheRun)
{
    const QJsonObject json = detectionToJson(makeResult());

    EXPECT_EQ(json[QStringLiteral("input")].toString(), QStringLiteral("processed"));
    EXPECT_EQ(json[QStringLiteral("frameIndex")].toInteger(), static_cast<qint64>(kFrameIndex));
    EXPECT_EQ(json[QStringLiteral("imageSize")].toArray(), (QJsonArray { kImageWidth, kImageHeight }));
    EXPECT_EQ(json[QStringLiteral("modelInputSize")].toArray(), (QJsonArray { kModelInput, kModelInput }));
    EXPECT_DOUBLE_EQ(json[QStringLiteral("ms")].toDouble(), kMilliseconds);
}

TEST_F(UtilsTest, GivenAResult_WhenWritingJson_ThenEachBoxHasLabelScoreAndPixels)
{
    const QJsonArray detections = detectionToJson(makeResult())[QStringLiteral("detections")].toArray();
    ASSERT_EQ(detections.size(), 2);

    const QJsonObject first = detections.at(0).toObject();
    EXPECT_EQ(first[QStringLiteral("label")].toString(), QStringLiteral("person"));
    EXPECT_EQ(first[QStringLiteral("classId")].toInt(), kPersonId);
    EXPECT_FLOAT_EQ(static_cast<float>(first[QStringLiteral("score")].toDouble()), kPersonScore);
    EXPECT_EQ(first[QStringLiteral("box")].toArray(),
        (QJsonArray { kBoxX, kBoxY, kBoxWidth, kBoxHeight }));
}

TEST_F(UtilsTest, GivenNoDetections_WhenWritingJson_ThenTheListIsEmpty)
{
    pipeline::DetectionResult result = makeResult();
    result.detections.clear();
    EXPECT_TRUE(detectionToJson(result)[QStringLiteral("detections")].toArray().isEmpty());
}

} // namespace pivision::display
