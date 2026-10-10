#include <pivision/display/UiConfig.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

namespace pivision::display {

QStringList knownViewModes()
{
    return { QStringLiteral("sideBySide"), QStringLiteral("overlay"),
        QStringLiteral("pictureInPicture"), QStringLiteral("detection") };
}

std::optional<UiConfig> parseUiConfig(const QByteArray &json, QString *error)
{
    const auto fail = [error](const QString &message) -> std::optional<UiConfig> {
        if (error)
            *error = message;
        return std::nullopt;
    };

    QJsonParseError parseError;
    const QJsonObject root = QJsonDocument::fromJson(json, &parseError).object();
    if (parseError.error != QJsonParseError::NoError)
        return fail(QStringLiteral("%1 at offset %2").arg(parseError.errorString()).arg(parseError.offset));

    UiConfig config;
    const QJsonArray buttons = root.value(QStringLiteral("modeButtons")).toArray();
    if (buttons.isEmpty())
        return fail(QStringLiteral("modeButtons needs at least one mode"));
    for (const QJsonValue &button : buttons) {
        const QString mode = button.toString();
        if (!knownViewModes().contains(mode))
            return fail(QStringLiteral("modeButtons: unknown mode \"%1\"").arg(mode));
        if (config.modeButtons.contains(mode))
            return fail(QStringLiteral("modeButtons: \"%1\" listed twice").arg(mode));
        config.modeButtons.append(mode);
    }

    config.defaultMode = root.value(QStringLiteral("defaultMode")).toString();
    if (!config.modeButtons.contains(config.defaultMode))
        return fail(QStringLiteral("defaultMode must be one of modeButtons"));

    return config;
}

} // namespace pivision::display
