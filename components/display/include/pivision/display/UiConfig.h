#ifndef PIVISION_DISPLAY_UICONFIG_H
#define PIVISION_DISPLAY_UICONFIG_H

#include <optional>

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace pivision::display {

// App layout settings, from app/config/ui.json (next to stages.json):
//
//   { "modeButtons": ["sideBySide", "overlay", "pictureInPicture", "detection"],
//     "defaultMode": "overlay" }
//
// modeButtons: the compare-mode buttons in the toolbar, left to right. Leave
// one out to hide that mode. defaultMode: the mode the app starts in.
struct UiConfig {
    QStringList modeButtons;
    QString defaultMode;
};

// The compare-mode names the files may use, as CompareView.qml knows them.
QStringList knownViewModes();

// Reads ui.json. The file ships inside the app, so anything wrong with it is
// an error: invalid JSON, an unknown or repeated mode, a default mode that
// has no button. Returns nothing and sets `error` if so.
std::optional<UiConfig> parseUiConfig(const QByteArray &json, QString *error = nullptr);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_UICONFIG_H
