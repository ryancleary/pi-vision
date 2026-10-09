#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <pivision/snapshot/Export.h>

namespace pivision::snapshot {

class ExportTest : public ::testing::Test {
protected:
    // Makes <root>/<name>/file.txt.
    static void makeFolder(const QString &root, const QString &name)
    {
        QDir(root).mkpath(name);
        QFile file(QDir(root).filePath(name + QStringLiteral("/file.txt")));
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.write("data");
    }
};

TEST_F(ExportTest, CopiesOnlyWhatIsNew)
{
    QTemporaryDir from;
    QTemporaryDir to;
    makeFolder(from.path(), QStringLiteral("000001_a"));
    makeFolder(from.path(), QStringLiteral("000002_b"));
    makeFolder(from.path(), QStringLiteral("lost+found")); // not a capture or dump

    ExportResult first = exportNumberedFolders(from.path(), to.path());
    ASSERT_TRUE(first.ok()) << first.error.toStdString();
    EXPECT_EQ(first.copied, 2);
    EXPECT_TRUE(QFile::exists(to.filePath(QStringLiteral("000002_b/file.txt"))));
    EXPECT_FALSE(QDir(to.filePath(QStringLiteral("lost+found"))).exists());

    makeFolder(from.path(), QStringLiteral("000003_c"));
    ExportResult second = exportNumberedFolders(from.path(), to.path());
    EXPECT_EQ(second.copied, 1);
    EXPECT_EQ(second.skipped, 2);
}

TEST_F(ExportTest, MissingSourceCopiesNothing)
{
    QTemporaryDir to;
    const ExportResult result = exportNumberedFolders(QStringLiteral("/nonexistent/pivision"), to.path());
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(result.copied, 0);
}

TEST_F(ExportTest, UnwritableDestinationReportsAnError)
{
    QTemporaryDir from;
    makeFolder(from.path(), QStringLiteral("000001_a"));
    const ExportResult result = exportNumberedFolders(from.path(), QStringLiteral("/proc/pivision"));
    EXPECT_FALSE(result.ok());
}

TEST_F(ExportTest, MountPointsDirectlyBelowRoot)
{
    const QStringList mounts { QStringLiteral("/"), QStringLiteral("/run/media"),
        QStringLiteral("/run/media/sdb1"), QStringLiteral("/run/media/sda1"),
        QStringLiteral("/run/media/sda1/nested"), QStringLiteral("/run/mediafoo/x") };
    EXPECT_EQ(mountPointsUnder(QStringLiteral("/run/media/"), mounts),
        (QStringList { QStringLiteral("/run/media/sda1"), QStringLiteral("/run/media/sdb1") }));
}

} // namespace pivision::snapshot
