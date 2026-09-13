#include "Utils.h"

#include <cstdio>

#include <QDateTime>
#include <QMutexLocker>

namespace pivision::logging {

// One instance for the whole process; the handler can run on any thread.
LogState &logState()
{
    static LogState instance;
    return instance;
}

const char *levelName(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "debug";
    case QtInfoMsg:
        return "info";
    case QtWarningMsg:
        return "warning";
    case QtCriticalMsg:
        return "critical";
    case QtFatalMsg:
        return "fatal";
    }
    return "unknown";
}

// syslog priorities, which systemd reads from a "<N>" line prefix.
int syslogPriority(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return 7;
    case QtInfoMsg:
        return 6;
    case QtWarningMsg:
        return 4;
    case QtCriticalMsg:
        return 3;
    case QtFatalMsg:
        return 2;
    }
    return 6;
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    const char *category = context.category ? context.category : "default";
    const QByteArray body = QByteArray(levelName(type)) + ' ' + category + ": " + message.toUtf8();
    const QByteArray stamped
        = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")).toUtf8()
        + ' ' + body + '\n';

    LogState &s = logState();
    QMutexLocker lock(&s.mutex);

    if (s.journal) {
        // The journal records the time and, from the prefix, the level.
        std::fprintf(stderr, "<%d>%s: %s\n", syslogPriority(type), category,
            message.toUtf8().constData());
    } else {
        std::fputs(stamped.constData(), stderr);
    }
    std::fflush(stderr);

    if (s.file.isOpen()) {
        s.file.write(stamped);
        s.file.flush(); // keep the file complete if the app crashes next
    }
}


} // namespace pivision::logging
