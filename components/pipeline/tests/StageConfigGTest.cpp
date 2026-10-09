#include <gtest/gtest.h>

#include <QByteArray>
#include <QString>

#include <pivision/pipeline/StageConfig.h>

namespace pivision::pipeline {

namespace {

const QByteArray kValid = R"({
  "stages": [
    { "id": "grayscale", "label": "Grayscale", "enabled": true },
    { "id": "blur", "label": "Blur",
      "parameters": [ { "name": "size", "label": "Kernel size",
                        "value": 5, "min": 3, "max": 15, "step": 2 } ] }
  ]
})";

QString errorFor(const QByteArray &json)
{
    QString error;
    EXPECT_FALSE(parseStageConfig(json, &error).has_value()) << json.toStdString();
    return error;
}

} // namespace

TEST(StageConfig, ReadsStagesInOrderWithParameters)
{
    QString error;
    const auto stages = parseStageConfig(kValid, &error);
    ASSERT_TRUE(stages.has_value()) << error.toStdString();
    ASSERT_EQ(stages->size(), 2);

    EXPECT_EQ(stages->at(0).id, QStringLiteral("grayscale"));
    EXPECT_TRUE(stages->at(0).enabled);
    EXPECT_TRUE(stages->at(0).parameters.isEmpty());

    const auto &blur = stages->at(1);
    EXPECT_EQ(blur.id, QStringLiteral("blur"));
    EXPECT_FALSE(blur.enabled); // defaults to off
    ASSERT_EQ(blur.parameters.size(), 1);
    EXPECT_EQ(blur.parameters[0].name, QStringLiteral("size"));
    EXPECT_EQ(blur.parameters[0].value, 5.0);
    EXPECT_EQ(blur.parameters[0].minimum, 3.0);
    EXPECT_EQ(blur.parameters[0].maximum, 15.0);
    EXPECT_EQ(blur.parameters[0].step, 2.0);
}

TEST(StageConfig, RejectsBrokenJson)
{
    EXPECT_FALSE(errorFor(R"({ "stages": [ )").isEmpty());
    EXPECT_TRUE(errorFor(R"([1, 2])").contains(QStringLiteral("stages")));
}

TEST(StageConfig, RejectsUnknownAndDuplicateStages)
{
    EXPECT_TRUE(errorFor(R"({ "stages": [ { "id": "sharpen" } ] })").contains(QStringLiteral("sharpen")));
    EXPECT_TRUE(errorFor(R"({ "stages": [ { "id": "blur" }, { "id": "blur" } ] })")
                    .contains(QStringLiteral("twice")));
}

TEST(StageConfig, RejectsBadParameters)
{
    // A parameter the stage doesn't have.
    EXPECT_TRUE(errorFor(R"({ "stages": [ { "id": "blur", "parameters": [
        { "name": "radius", "value": 3, "min": 1, "max": 9 } ] } ] })")
                    .contains(QStringLiteral("radius")));
    // Missing range.
    EXPECT_FALSE(errorFor(R"({ "stages": [ { "id": "blur", "parameters": [
        { "name": "size", "value": 5 } ] } ] })").isEmpty());
    // Value outside its own range.
    EXPECT_FALSE(errorFor(R"({ "stages": [ { "id": "blur", "parameters": [
        { "name": "size", "value": 99, "min": 3, "max": 15 } ] } ] })").isEmpty());
}

} // namespace pivision::pipeline
