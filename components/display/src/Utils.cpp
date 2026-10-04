#include "Utils.h"

#include <QJsonDocument>
#include <QJsonParseError>


namespace pivision::display {

QJsonArray sizeToJson(const QSize &size)
{
    return QJsonArray { size.width(), size.height() };
}

QJsonArray stagesToJson(const QList<pipeline::StageConfig> &stages)
{
    QJsonArray list;
    for (const pipeline::StageConfig &stage : stages) {
        QJsonObject parameters;
        for (const pipeline::StageParameterConfig &parameter : stage.parameters)
            parameters.insert(parameter.name, parameter.value);
        list.append(QJsonObject {
            { QStringLiteral("id"), stage.id },
            { QStringLiteral("label"), stage.label },
            { QStringLiteral("enabled"), stage.enabled },
            { QStringLiteral("parameters"), parameters },
        });
    }
    return list;
}

QString classLabel(const pipeline::DetectionResult &result, int classId)
{
    const auto index = static_cast<size_t>(classId);
    if (classId >= 0 && result.classNames && index < result.classNames->size())
        return QString::fromStdString(result.classNames->at(index));
    return QString::number(classId);
}

QJsonObject detectionToJson(const pipeline::DetectionResult &result)
{
    QJsonArray boxes;
    for (const detection::Detection &detection : result.detections) {
        boxes.append(QJsonObject {
            { QStringLiteral("label"), classLabel(result, detection.classId) },
            { QStringLiteral("classId"), detection.classId },
            { QStringLiteral("score"), detection.score },
            // [x, y, width, height] in the pixels of imageSize.
            { QStringLiteral("box"),
                QJsonArray { detection.box.x, detection.box.y, detection.box.width,
                    detection.box.height } },
        });
    }
    return QJsonObject {
        { QStringLiteral("input"),
            result.input == pipeline::DetectionInput::Processed ? QStringLiteral("processed")
                                                                : QStringLiteral("raw") },
        // Detection runs behind the camera, so this is usually an earlier
        // frame than the one in raw.png.
        { QStringLiteral("frameIndex"), static_cast<qint64>(result.frameIndex) },
        { QStringLiteral("imageSize"), sizeToJson(result.image.size()) },
        { QStringLiteral("modelInputSize"), sizeToJson(result.modelInputSize) },
        { QStringLiteral("ms"), result.milliseconds },
        { QStringLiteral("detections"), boxes },
    };
}

std::optional<QJsonObject> jsonObject(const QByteArray &json, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error)
            *error = QStringLiteral("%1 at offset %2").arg(parseError.errorString()).arg(parseError.offset);
        return std::nullopt;
    }
    return document.object();
}

std::optional<UiConfig> readUiConfig(const QJsonObject &root, UiConfig config, bool modesRequired,
    QString *error)
{
    const auto fail = [error](const QString &message) -> std::optional<UiConfig> {
        if (error)
            *error = message;
        return std::nullopt;
    };

    const QString buttonsKey = QStringLiteral("modeButtons");
    if (modesRequired || root.contains(buttonsKey)) {
        const QJsonArray buttons = root.value(buttonsKey).toArray();
        if (buttons.isEmpty())
            return fail(QStringLiteral("modeButtons needs at least one mode"));
        config.modeButtons.clear();
        for (const QJsonValue &button : buttons) {
            const QString mode = button.toString();
            if (!knownViewModes().contains(mode))
                return fail(QStringLiteral("modeButtons: unknown mode \"%1\"").arg(mode));
            if (config.modeButtons.contains(mode))
                return fail(QStringLiteral("modeButtons: \"%1\" listed twice").arg(mode));
            config.modeButtons.append(mode);
        }
    }

    const QString defaultKey = QStringLiteral("defaultMode");
    if (modesRequired || root.contains(defaultKey))
        config.defaultMode = root.value(defaultKey).toString();
    if (!config.modeButtons.contains(config.defaultMode))
        return fail(QStringLiteral("defaultMode must be one of modeButtons"));

    const QString toolbarKey = QStringLiteral("toolbar");
    if (root.contains(toolbarKey)) {
        const QString toolbar = root.value(toolbarKey).toString();
        if (toolbar == QStringLiteral("top"))
            config.toolbar = ToolbarPosition::Top;
        else if (toolbar == QStringLiteral("bottom"))
            config.toolbar = ToolbarPosition::Bottom;
        else
            return fail(QStringLiteral("toolbar must be \"top\" or \"bottom\""));
    }

    const QString insetKey = QStringLiteral("topInset");
    if (root.contains(insetKey)) {
        const QJsonValue inset = root.value(insetKey);
        // toInt() returns the fallback for anything that isn't a whole number.
        const int notAWholeNumber = -1;
        config.topInset = inset.toInt(notAWholeNumber);
        if (!inset.isDouble() || config.topInset < 0)
            return fail(QStringLiteral("topInset must be a whole number of pixels, 0 or more"));
    }

    return config;
}

std::optional<ScenarioStep> readScenarioStep(const QJsonObject &object, QString *error)
{
    const auto fail = [error](const QString &message) -> std::optional<ScenarioStep> {
        if (error)
            *error = message;
        return std::nullopt;
    };
    // A positive number of seconds under `key`.
    const auto seconds = [&object](const QString &key) {
        const QJsonValue value = object.value(key);
        return value.isDouble() && value.toDouble() > 0.0 ? value.toDouble() : 0.0;
    };

    ScenarioStep step;
    step.text = QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
    const QString action = object.value(QStringLiteral("do")).toString();

    if (action == QStringLiteral("mode")) {
        step.action = ScenarioStep::Action::Mode;
        step.value = object.value(QStringLiteral("value")).toString();
        if (!knownViewModes().contains(step.value))
            return fail(QStringLiteral("mode must be one of %1").arg(knownViewModes().join(QStringLiteral(", "))));
    } else if (action == QStringLiteral("stages")) {
        step.action = ScenarioStep::Action::Stages;
        const QJsonValue enabled = object.value(QStringLiteral("enabled"));
        if (!enabled.isArray())
            return fail(QStringLiteral("stages needs an \"enabled\" list (empty for none)"));
        for (const QJsonValue &id : enabled.toArray())
            step.stages.append(id.toString());
    } else if (action == QStringLiteral("detectionInput")) {
        step.action = ScenarioStep::Action::DetectionInput;
        step.value = object.value(QStringLiteral("value")).toString();
        if (step.value != QStringLiteral("raw") && step.value != QStringLiteral("processed"))
            return fail(QStringLiteral("detectionInput must be \"raw\" or \"processed\""));
    } else if (action == QStringLiteral("wait")) {
        step.action = ScenarioStep::Action::Wait;
        step.seconds = seconds(QStringLiteral("seconds"));
        if (step.seconds <= 0.0)
            return fail(QStringLiteral("wait needs \"seconds\" above 0"));
    } else if (action == QStringLiteral("capture")) {
        step.action = ScenarioStep::Action::Capture;
        step.label = object.value(QStringLiteral("label")).toString();
        if (step.label.isEmpty())
            return fail(QStringLiteral("capture needs a \"label\""));
    } else if (action == QStringLiteral("expectFrames")) {
        step.action = ScenarioStep::Action::ExpectFrames;
        step.atLeast = object.value(QStringLiteral("atLeast")).toInt();
        step.seconds = seconds(QStringLiteral("withinSeconds"));
        if (step.atLeast <= 0 || step.seconds <= 0.0)
            return fail(QStringLiteral("expectFrames needs \"atLeast\" and \"withinSeconds\" above 0"));
    } else if (action == QStringLiteral("expectDetection")) {
        step.action = ScenarioStep::Action::ExpectDetection;
        step.label = object.value(QStringLiteral("label")).toString();
        step.seconds = seconds(QStringLiteral("withinSeconds"));
        if (step.label.isEmpty() || step.seconds <= 0.0)
            return fail(QStringLiteral("expectDetection needs a \"label\" and \"withinSeconds\" above 0"));
    } else {
        return fail(QStringLiteral("unknown \"do\": \"%1\"; one of %2")
                        .arg(action, knownScenarioActions().join(QStringLiteral(", "))));
    }
    return step;
}

QVariantMap sourceEntry(const QString &label, const QString &spec)
{
    return { { QStringLiteral("label"), label }, { QStringLiteral("spec"), spec } };
}

} // namespace pivision::display
