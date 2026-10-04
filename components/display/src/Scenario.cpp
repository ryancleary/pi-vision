#include <pivision/display/Scenario.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <pivision/display/UiConfig.h>

#include "Utils.h"

namespace pivision::display {

QStringList knownScenarioActions()
{
    return { QStringLiteral("mode"), QStringLiteral("stages"), QStringLiteral("detectionInput"),
        QStringLiteral("wait"), QStringLiteral("capture"), QStringLiteral("expectFrames"),
        QStringLiteral("expectDetection") };
}

std::optional<Scenario> parseScenario(const QByteArray &json, QString *error)
{
    const auto root = jsonObject(json, error);
    if (!root)
        return std::nullopt;

    Scenario scenario;
    scenario.about = root->value(QStringLiteral("about")).toString();
    const QJsonArray steps = root->value(QStringLiteral("steps")).toArray();
    if (steps.isEmpty()) {
        if (error)
            *error = QStringLiteral("steps needs at least one step");
        return std::nullopt;
    }

    for (qsizetype i = 0; i < steps.size(); ++i) {
        const auto step = readScenarioStep(steps.at(i).toObject(), error);
        if (!step) {
            if (error)
                *error = QStringLiteral("step %1: %2").arg(i + 1).arg(*error);
            return std::nullopt;
        }
        scenario.steps.append(*step);
    }
    return scenario;
}

} // namespace pivision::display
