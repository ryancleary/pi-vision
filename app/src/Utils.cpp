#include "Utils.h"

#include <cstdio>
#include <cstdlib>

#include <QRegularExpression>

namespace pivision::app {

int usageError(const QString &message)
{
    std::fprintf(stderr, "pivision: %s\nTry --help.\n", qPrintable(message));
    return EXIT_FAILURE;
}

StorageDefaults storageDefaults()
{
    // Set by the desktop CMake presets; empty in device builds.
    const QString devCaptures = QStringLiteral(PIVISION_DEV_CAPTURE_DIR);
    if (!devCaptures.isEmpty())
        return { devCaptures, 0, QString(),
            QStringLiteral("/run/media/%1").arg(qEnvironmentVariable("USER")) };
    return { QStringLiteral("/var/crash/captures"), 10, QStringLiteral("/var/crash"),
        QStringLiteral("/run/media") };
}

std::optional<QSize> parseSize(const QString &text)
{
    static const QRegularExpression pattern(QStringLiteral("^(\\d+)x(\\d+)$"));
    const auto match = pattern.match(text);
    if (!match.hasMatch())
        return std::nullopt;
    const QSize size(match.captured(1).toInt(), match.captured(2).toInt());
    return size.isEmpty() ? std::nullopt : std::optional<QSize>(size);
}

} // namespace pivision::app
