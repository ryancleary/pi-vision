#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QString>
#include <QTemporaryDir>

#include <pivision/logging/Logging.h>

namespace pivision::logging {

namespace {

QString readAll(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    return QString::fromUtf8(file.readAll());
}

} // namespace

TEST(Logging, WritesLevelCategoryAndMessageToTheLogFile)
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("nested/pivision.log"));
    ASSERT_TRUE(install(path)); // creates the missing directory

    qCInfo(lcPipeline) << "Opened" << 42;
    qCWarning(lcCapture, "camera %s gone", "/dev/video0");

    const QString log = readAll(path);
    EXPECT_TRUE(log.contains(QStringLiteral("info pivision.pipeline: Opened 42"))) << log.toStdString();
    EXPECT_TRUE(log.contains(QStringLiteral("warning pivision.capture: camera /dev/video0 gone")))
        << log.toStdString();
}

TEST(Logging, EachInstallStartsAFreshFile)
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("pivision.log"));

    ASSERT_TRUE(install(path));
    qCInfo(lcApp) << "first session";
    ASSERT_TRUE(install(path));
    qCInfo(lcApp) << "second session";

    const QString log = readAll(path);
    EXPECT_FALSE(log.contains(QStringLiteral("first session")));
    EXPECT_TRUE(log.contains(QStringLiteral("second session")));
}

TEST(Logging, DebugIsOffUntilVerbose)
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("pivision.log"));
    ASSERT_TRUE(install(path));

    qCDebug(lcDisplay) << "hidden";
    enableVerbose();
    qCDebug(lcDisplay) << "shown";

    const QString log = readAll(path);
    EXPECT_FALSE(log.contains(QStringLiteral("hidden")));
    EXPECT_TRUE(log.contains(QStringLiteral("debug pivision.display: shown")));
    QLoggingCategory::setFilterRules({});
}

TEST(Logging, UnwritableFileFailsButLoggingContinues)
{
    EXPECT_FALSE(install(QStringLiteral("/proc/pivision-cannot-write/pivision.log")));
    qCInfo(lcApp) << "still logs to stderr";
    install(); // back to stderr only for any later tests
}

} // namespace pivision::logging
