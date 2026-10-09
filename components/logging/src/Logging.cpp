#include <pivision/logging/Logging.h>

#include <cstdio>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

namespace pivision::logging {

Q_LOGGING_CATEGORY(lcApp, "pivision.app", QtInfoMsg)
Q_LOGGING_CATEGORY(lcCapture, "pivision.capture", QtInfoMsg)
Q_LOGGING_CATEGORY(lcPipeline, "pivision.pipeline", QtInfoMsg)
Q_LOGGING_CATEGORY(lcDisplay, "pivision.display", QtInfoMsg)

namespace {

    struct State {
        QMutex mutex;
        QFile file;
        bool journal = false;
    };

    State &state()
    {
        static State instance;
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

    void handler(QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        const char *category = context.category ? context.category : "default";
        const QByteArray body = QByteArray(levelName(type)) + ' ' + category + ": " + message.toUtf8();
        const QByteArray stamped
            = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")).toUtf8()
            + ' ' + body + '\n';

        State &s = state();
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

} // namespace

bool install(const QString &logFile)
{
    State &s = state();
    bool fileOk = true;
    {
        QMutexLocker lock(&s.mutex);
        // systemd sets JOURNAL_STREAM when stdout/stderr go to the journal.
        s.journal = qEnvironmentVariableIsSet("JOURNAL_STREAM");

        if (s.file.isOpen())
            s.file.close();
        if (!logFile.isEmpty()) {
            QDir().mkpath(QFileInfo(logFile).absolutePath());
            s.file.setFileName(logFile);
            fileOk = s.file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
        }
    }

    qInstallMessageHandler(handler);
    if (!fileOk)
        qCWarning(lcApp) << "Could not open log file" << logFile << "-" << s.file.errorString();
    return fileOk;
}

void enableVerbose()
{
    QLoggingCategory::setFilterRules(QStringLiteral("pivision.*.debug=true"));
}

} // namespace pivision::logging
