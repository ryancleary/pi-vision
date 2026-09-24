#ifndef PIVISION_APP_UTILS_H
#define PIVISION_APP_UTILS_H

#include <optional>

#include <QSize>
#include <QString>

namespace pivision::app {

// Prints "pivision: <message>" and a --help hint to stderr; returns the exit code.
int usageError(const QString &message);

// Where files are, unless options say otherwise.
//   Development builds: captures in a folder in the repo, all kept; no crash
//   dumps; USB sticks where the desktop mounts them (udisks).
//   On the device: captures on the crash partition, which is small (64 MB,
//   shared with crash dumps), so only the newest 10; sticks where our udev
//   rule mounts them.
// Models: the repo's assets/models in development builds; the image installs
// them in /usr/share/pivision/models.
struct StorageDefaults {
    QString captureDirectory;
    int captureKeep;
    QString crashDirectory;
    QString usbRoot;
    QString modelDirectory;
};

// The detection model used unless --model names another description file.
constexpr const char *kDefaultModelDescription = "nanodet-plus-m-1.5x-416.json";

StorageDefaults storageDefaults();

// "640x480" -> QSize(640, 480); nothing for any other form or an empty size.
std::optional<QSize> parseSize(const QString &text);

} // namespace pivision::app

#endif // PIVISION_APP_UTILS_H
