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

QVariantMap sourceEntry(const QString &label, const QString &spec)
{
    return { { QStringLiteral("label"), label }, { QStringLiteral("spec"), spec } };
}

} // namespace pivision::display
