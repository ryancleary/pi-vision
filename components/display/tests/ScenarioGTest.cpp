#include <gtest/gtest.h>

#include <QDir>
#include <QFile>

#include <pivision/display/Scenario.h>

namespace pivision::display {

class ScenarioTest : public ::testing::Test {
protected:
    // Parses `json`, expecting it to fail, and returns the error.
    static QString errorFor(const QByteArray &json)
    {
        QString error;
        EXPECT_FALSE(parseScenario(json, &error).has_value()) << json.toStdString();
        return error;
    }
};

TEST_F(ScenarioTest, GivenTheShippedScenarios_WhenParsing_ThenEachIsValid)
{
    const QDir directory(QStringLiteral(PIVISION_SYSTEM_TESTS));
    const QStringList files = directory.entryList({ QStringLiteral("*.json") });
    ASSERT_FALSE(files.isEmpty()) << PIVISION_SYSTEM_TESTS;
    for (const QString &name : files) {
        QFile file(directory.filePath(name));
        ASSERT_TRUE(file.open(QIODevice::ReadOnly)) << name.toStdString();
        QString error;
        EXPECT_TRUE(parseScenario(file.readAll(), &error).has_value())
            << name.toStdString() << ": " << error.toStdString();
    }
}

TEST_F(ScenarioTest, GivenOneStepOfEachKind_WhenParsing_ThenTheyAreReadInOrder)
{
    // Distinct values, so each one can only have come from its own field.
    constexpr int kFrames = 30;
    constexpr double kWaitSeconds = 2.5;
    constexpr double kLimitSeconds = 9.0;
    const auto scenario = parseScenario(QStringLiteral(R"({ "about": "test", "steps": [
        { "do": "mode", "value": "overlay" },
        { "do": "stages", "enabled": ["blur", "edges"] },
        { "do": "detectionInput", "value": "processed" },
        { "do": "wait", "seconds": %1, "about": "ignored" },
        { "do": "capture", "label": "first" },
        { "do": "expectFrames", "atLeast": %2, "withinSeconds": %3 },
        { "do": "expectDetection", "label": "person", "withinSeconds": %3 } ] })")
                                            .arg(kWaitSeconds)
                                            .arg(kFrames)
                                            .arg(kLimitSeconds)
                                            .toUtf8());
    ASSERT_TRUE(scenario.has_value());
    ASSERT_EQ(scenario->steps.size(), knownScenarioActions().size());
    EXPECT_EQ(scenario->about, QStringLiteral("test"));

    const auto &steps = scenario->steps;
    EXPECT_EQ(steps[0].action, ScenarioStep::Action::Mode);
    EXPECT_EQ(steps[0].value, QStringLiteral("overlay"));
    EXPECT_EQ(steps[1].stages, (QStringList { QStringLiteral("blur"), QStringLiteral("edges") }));
    EXPECT_EQ(steps[2].value, QStringLiteral("processed"));
    EXPECT_DOUBLE_EQ(steps[3].seconds, kWaitSeconds);
    EXPECT_EQ(steps[4].label, QStringLiteral("first"));
    EXPECT_EQ(steps[5].atLeast, kFrames);
    EXPECT_DOUBLE_EQ(steps[5].seconds, kLimitSeconds);
    EXPECT_EQ(steps[6].label, QStringLiteral("person"));
}

TEST_F(ScenarioTest, GivenNoStagesListed_WhenParsing_ThenAllAreSwitchedOff)
{
    const auto scenario = parseScenario(R"({ "steps": [ { "do": "stages", "enabled": [] } ] })");
    ASSERT_TRUE(scenario.has_value());
    EXPECT_TRUE(scenario->steps[0].stages.isEmpty());
}

TEST_F(ScenarioTest, GivenAnUnknownAction_WhenParsing_ThenTheErrorNamesItAndTheStep)
{
    const QString error = errorFor(R"({ "steps": [ { "do": "wait", "seconds": 1 },
                                                    { "do": "jump" } ] })");
    EXPECT_TRUE(error.contains(QStringLiteral("jump")));
    EXPECT_TRUE(error.contains(QStringLiteral("step 2")));
}

TEST_F(ScenarioTest, GivenMissingOrWrongFields_WhenParsing_ThenEachIsRejected)
{
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "mode", "value": "split" } ] })").isEmpty());
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "stages" } ] })").isEmpty());
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "detectionInput", "value": "both" } ] })").isEmpty());
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "wait", "seconds": 0 } ] })").isEmpty());
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "capture" } ] })").isEmpty());
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "expectFrames", "atLeast": 10 } ] })").isEmpty());
    EXPECT_FALSE(errorFor(R"({ "steps": [ { "do": "expectDetection", "withinSeconds": 5 } ] })").isEmpty());
}

TEST_F(ScenarioTest, GivenNoSteps_WhenParsing_ThenItIsRejected)
{
    EXPECT_TRUE(errorFor(R"({ "steps": [] })").contains(QStringLiteral("steps")));
}

} // namespace pivision::display
