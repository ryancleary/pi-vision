#ifndef PIVISION_DISPLAY_UICONFIG_H
#define PIVISION_DISPLAY_UICONFIG_H

#include <optional>

#include <QByteArray>
#include <QString>
#include <QStringList>

namespace pivision::display {

// Where the toolbar sits.
enum class ToolbarPosition {
    Top,
    Bottom,
};

// App layout settings, from app/config/ui.json (next to stages.json):
//
//   { "modeButtons": ["sideBySide", "overlay", "pictureInPicture", "detection"],
//     "defaultMode": "overlay",
//     "toolbar": "top",
//     "topInset": 0 }
//
// modeButtons: the compare-mode buttons in the toolbar, left to right. Leave
// one out to hide that mode. defaultMode: the mode the app starts in.
// toolbar: "top" or "bottom" (optional, default top). topInset: pixels at
// the top of the screen kept free of controls (optional, default 0); a
// status line fills them. For touch panels that can't sense their top edge.
//
// A device can override any of these with a second file (see
// applyUiOverrides); the display profiles in the image install one.
// "about" keys are ignored, so files can say why a value is set.
struct UiConfig {
    QStringList modeButtons;
    QString defaultMode;
    ToolbarPosition toolbar = ToolbarPosition::Top;
    int topInset = 0;
};

// The compare-mode names the files may use, as CompareView.qml knows them.
QStringList knownViewModes();

// Reads ui.json. The file ships inside the app, so anything wrong with it is
// an error: invalid JSON, an unknown or repeated mode, a default mode that
// has no button, an unknown toolbar position, a negative inset. Returns
// nothing and sets `error` if so.
std::optional<UiConfig> parseUiConfig(const QByteArray &json, QString *error = nullptr);

// Reads an override file: any of ui.json's keys, each replacing `base`'s
// value; keys left out keep it. The result is checked the same way.
std::optional<UiConfig> applyUiOverrides(const UiConfig &base, const QByteArray &json,
    QString *error = nullptr);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_UICONFIG_H
