#ifndef PIVISION_LOGGING_UTILS_H
#define PIVISION_LOGGING_UTILS_H

#include <QFile>
#include <QMutex>
#include <QString>
#include <QtGlobal>

// Internals of the logging component: the shared state and the Qt message
// handler that install() puts in place.
namespace pivision::logging {

struct LogState {
    QMutex mutex;
    QFile file;
    bool journal = false; // stderr goes to the systemd journal
};

LogState &logState();

// "debug", "info", ... as written in the log.
const char *levelName(QtMsgType type);

// syslog priority for the "<N>" prefix systemd reads from each journal line.
int syslogPriority(QtMsgType type);

// Writes each message to stderr (journal format under systemd) and the log file.
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message);

} // namespace pivision::logging

#endif // PIVISION_LOGGING_UTILS_H
