#include <gtest/gtest.h>

#include <QDir>
#include <QFile>

#include <pivision/display/UiConfig.h>

namespace pivision::display {

class UiConfigTest : public ::testing::Test {
protected:
    // Any inset works; this one is easy to spot.
    static constexpr int kInset = 60;

    // A valid built-in file with two modes and no layout keys.
    static UiConfig base()
    {
        const auto config = parseUiConfig(R"({ "modeButtons": ["sideBySide", "overlay"],
                                               "defaultMode": "overlay" })");
        EXPECT_TRUE(config.has_value());
        return config.value_or(UiConfig());
    }

    // Applies `json` as overrides to base(), expecting it to fail, and returns the error.
    static QString overrideErrorFor(const QByteArray &json)
    {
        QString error;
        EXPECT_FALSE(applyUiOverrides(base(), json, &error).has_value()) << json.toStdString();
        return error;
    }

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

TEST_F(UiConfigTest, GivenNoLayoutKeys_WhenParsing_ThenToolbarIsAtTopWithNoInset)
{
    const UiConfig config = base();
    EXPECT_EQ(config.toolbar, ToolbarPosition::Top);
    EXPECT_EQ(config.topInset, 0);
}

TEST_F(UiConfigTest, GivenABottomToolbarAndAnInset_WhenParsing_ThenBothAreRead)
{
    const auto config = parseUiConfig(QStringLiteral(R"({ "modeButtons": ["overlay"],
        "defaultMode": "overlay", "toolbar": "bottom", "topInset": %1 })").arg(kInset).toUtf8());
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->toolbar, ToolbarPosition::Bottom);
    EXPECT_EQ(config->topInset, kInset);
}

TEST_F(UiConfigTest, GivenAnUnknownToolbarPosition_WhenParsing_ThenItIsRejected)
{
    EXPECT_TRUE(errorFor(R"({ "modeButtons": ["overlay"], "defaultMode": "overlay",
                               "toolbar": "left" })")
                    .contains(QStringLiteral("toolbar")));
}

TEST_F(UiConfigTest, GivenANegativeOrFractionalInset_WhenParsing_ThenItIsRejected)
{
    EXPECT_TRUE(errorFor(R"({ "modeButtons": ["overlay"], "defaultMode": "overlay",
                               "topInset": -1 })")
                    .contains(QStringLiteral("topInset")));
    EXPECT_TRUE(errorFor(R"({ "modeButtons": ["overlay"], "defaultMode": "overlay",
                               "topInset": 2.5 })")
                    .contains(QStringLiteral("topInset")));
}

TEST_F(UiConfigTest, GivenOnlyLayoutOverrides_WhenApplying_ThenTheModesAreKept)
{
    const auto config = applyUiOverrides(base(),
        QStringLiteral(R"({ "about": "why", "toolbar": "bottom", "topInset": %1 })").arg(kInset).toUtf8());
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->modeButtons, base().modeButtons);
    EXPECT_EQ(config->defaultMode, base().defaultMode);
    EXPECT_EQ(config->toolbar, ToolbarPosition::Bottom);
    EXPECT_EQ(config->topInset, kInset);
}

TEST_F(UiConfigTest, GivenAnOverrideDefaultWithoutAButton_WhenApplying_ThenItIsRejected)
{
    EXPECT_TRUE(overrideErrorFor(R"({ "defaultMode": "detection" })")
                    .contains(QStringLiteral("defaultMode")));
}

TEST_F(UiConfigTest, GivenTheShippedDisplayProfiles_WhenApplyingThem_ThenEachIsValid)
{
    const QDir profiles(QStringLiteral(PIVISION_DISPLAY_PROFILES));
    const QStringList names = profiles.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    ASSERT_FALSE(names.isEmpty()) << PIVISION_DISPLAY_PROFILES;
    for (const QString &name : names) {
        QFile file(profiles.filePath(name + QStringLiteral("/ui.json")));
        if (!file.exists())
            continue; // a profile may only add a touch rule
        ASSERT_TRUE(file.open(QIODevice::ReadOnly)) << name.toStdString();
        QString error;
        EXPECT_TRUE(applyUiOverrides(base(), file.readAll(), &error).has_value())
            << name.toStdString() << ": " << error.toStdString();
    }
}

} // namespace pivision::display
