#include <pivision/display/UiConfig.h>

#include "Utils.h"

namespace pivision::display {

QStringList knownViewModes()
{
    return { QStringLiteral("sideBySide"), QStringLiteral("overlay"),
        QStringLiteral("pictureInPicture"), QStringLiteral("detection") };
}

std::optional<UiConfig> parseUiConfig(const QByteArray &json, QString *error)
{
    const auto root = jsonObject(json, error);
    return root ? readUiConfig(*root, UiConfig(), true, error) : std::nullopt;
}

std::optional<UiConfig> applyUiOverrides(const UiConfig &base, const QByteArray &json, QString *error)
{
    const auto root = jsonObject(json, error);
    return root ? readUiConfig(*root, base, false, error) : std::nullopt;
}

} // namespace pivision::display
