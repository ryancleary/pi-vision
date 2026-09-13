#include <pivision/logging/Logging.h>

#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>

#include "Utils.h"

namespace pivision::logging {

Q_LOGGING_CATEGORY(lcApp, "pivision.app", QtInfoMsg)
Q_LOGGING_CATEGORY(lcCapture, "pivision.capture", QtInfoMsg)
Q_LOGGING_CATEGORY(lcPipeline, "pivision.pipeline", QtInfoMsg)
Q_LOGGING_CATEGORY(lcDisplay, "pivision.display", QtInfoMsg)

bool install(const QString &logFile)
{
    LogState &s = logState();
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

    qInstallMessageHandler(messageHandler);
    if (!fileOk)
        qCWarning(lcApp) << "Could not open log file" << logFile << "-" << s.file.errorString();
    return fileOk;
}

void enableVerbose()
{
    QLoggingCategory::setFilterRules(QStringLiteral("pivision.*.debug=true"));
}

} // namespace pivision::logging
