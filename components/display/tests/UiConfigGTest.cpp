#include <gtest/gtest.h>

#include <QFile>

#include <pivision/display/UiConfig.h>

namespace pivision::display {

class UiConfigTest : public ::testing::Test {
protected:
    // Parses `json`, expecting it to fail, and returns the error.
    static QString errorFor(const QByteArray &json)
    {
        QString error;
        EXPECT_FALSE(parseUiConfig(json, &error).has_value()) << json.toStdString();
        return error;
    }
};

TEST_F(UiConfigTest, GivenTheShippedUiJson_WhenParsing_ThenItIsValid)
{
    QFile file(QStringLiteral(PIVISION_UI_CONFIG));
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    QString error;
    EXPECT_TRUE(parseUiConfig(file.readAll(), &error).has_value()) << error.toStdString();
}

TEST_F(UiConfigTest, GivenButtonsAndADefault_WhenParsing_ThenTheOrderIsKept)
{
    const auto config = parseUiConfig(R"({ "modeButtons": ["pictureInPicture", "overlay"],
                                           "defaultMode": "overlay" })");
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->modeButtons,
        (QStringList { QStringLiteral("pictureInPicture"), QStringLiteral("overlay") }));
    EXPECT_EQ(config->defaultMode, QStringLiteral("overlay"));
}

TEST_F(UiConfigTest, GivenAnUnknownMode_WhenParsing_ThenAnErrorNamesIt)
{
    EXPECT_TRUE(errorFor(R"({ "modeButtons": ["overlay", "split"], "defaultMode": "overlay" })")
                    .contains(QStringLiteral("split")));
}

TEST_F(UiConfigTest, GivenARepeatedMode_WhenParsing_ThenItIsRejected)
{
    EXPECT_TRUE(errorFor(R"({ "modeButtons": ["overlay", "overlay"], "defaultMode": "overlay" })")
                    .contains(QStringLiteral("twice")));
}

TEST_F(UiConfigTest, GivenADefaultWithoutAButton_WhenParsing_ThenItIsRejected)
{
    EXPECT_TRUE(errorFor(R"({ "modeButtons": ["overlay"], "defaultMode": "sideBySide" })")
                    .contains(QStringLiteral("defaultMode")));
}

TEST_F(UiConfigTest, GivenBrokenJson_WhenParsing_ThenItIsRejected)
{
    EXPECT_FALSE(errorFor("{ \"modeButtons\": [").isEmpty());
}

} // namespace pivision::display
