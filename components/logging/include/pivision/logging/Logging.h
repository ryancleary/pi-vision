#ifndef PIVISION_LOGGING_LOGGING_H
#define PIVISION_LOGGING_LOGGING_H

#include <QLoggingCategory>
#include <QString>

namespace pivision::logging {

// One category per component. Debug messages are off by default; enable them
// with --verbose or QT_LOGGING_RULES, e.g. "pivision.pipeline.debug=true".
Q_DECLARE_LOGGING_CATEGORY(lcApp)
Q_DECLARE_LOGGING_CATEGORY(lcCapture)
Q_DECLARE_LOGGING_CATEGORY(lcPipeline)
Q_DECLARE_LOGGING_CATEGORY(lcDisplay)

// Routes all Qt log output through pi-vision's format:
//   2026-10-09 11:04:12.345 info pivision.pipeline: Opened Test pattern
// Under systemd (stderr connected to the journal), the time and level are left
// to the journal: each line is "<priority>category: message", so
// `journalctl -p warning` works.
//
// If `logFile` is not empty, every message is also written there. The file is
// truncated first, so it only ever holds the current session. Returns false if
// the file can't be opened; logging to stderr continues either way.
bool install(const QString &logFile = {});

// Turns on debug messages for every pi-vision category.
void enableVerbose();

} // namespace pivision::logging

#endif // PIVISION_LOGGING_LOGGING_H
