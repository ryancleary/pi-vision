#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <pivision/snapshot/SnapshotStore.h>

namespace pivision::snapshot {

class SnapshotStoreTest : public ::testing::Test {
protected:
    static QImage solidImage(int width, int height, Qt::GlobalColor color)
    {
        QImage image(width, height, QImage::Format_RGB888);
        image.fill(color);
        return image;
    }

    static SaveResult saveOne(const SnapshotStore &store)
    {
        return store.save(solidImage(64, 48, Qt::red), solidImage(32, 24, Qt::gray),
            QJsonObject { { QStringLiteral("source"), QStringLiteral("Test pattern") } });
    }
};

TEST_F(SnapshotStoreTest, SavesImagesAndInfo)
{
    QTemporaryDir root;
    const SnapshotStore store(root.filePath(QStringLiteral("captures")), 0);

    const SaveResult result = saveOne(store);
    ASSERT_TRUE(result.ok()) << result.error.toStdString();
    EXPECT_TRUE(result.name.startsWith(QStringLiteral("000001_")));

    const QDir folder(result.path);
    EXPECT_EQ(QImage(folder.filePath(QStringLiteral("raw.png"))).size(), QSize(64, 48));
    EXPECT_EQ(QImage(folder.filePath(QStringLiteral("processed.png"))).size(), QSize(32, 24));

    QFile json(folder.filePath(QStringLiteral("info.json")));
    ASSERT_TRUE(json.open(QIODevice::ReadOnly));
    const QJsonObject info = QJsonDocument::fromJson(json.readAll()).object();
    EXPECT_EQ(info.value(QStringLiteral("source")).toString(), QStringLiteral("Test pattern"));
    EXPECT_EQ(info.value(QStringLiteral("sequence")).toInt(), 1);
    EXPECT_FALSE(info.value(QStringLiteral("time")).toString().isEmpty());
}

TEST_F(SnapshotStoreTest, NumbersFollowTheNewestEvenAfterPruning)
{
    QTemporaryDir root;
    const SnapshotStore store(root.path(), 2);
    for (int i = 0; i < 4; ++i)
        ASSERT_TRUE(saveOne(store).ok());

    const QStringList names = store.list();
    ASSERT_EQ(names.size(), 2);
    EXPECT_TRUE(names.at(0).startsWith(QStringLiteral("000003_")));
    EXPECT_TRUE(names.at(1).startsWith(QStringLiteral("000004_")));
}

TEST_F(SnapshotStoreTest, KeepZeroKeepsEverything)
{
    QTemporaryDir root;
    const SnapshotStore store(root.path(), 0);
    for (int i = 0; i < 12; ++i)
        ASSERT_TRUE(saveOne(store).ok());
    EXPECT_EQ(store.list().size(), 12);
}

TEST_F(SnapshotStoreTest, OtherFilesAreLeftAlone)
{
    // On the Pi, captures share the crash partition with crash dumps.
    QTemporaryDir root;
    QDir(root.path()).mkdir(QStringLiteral("lost+found"));
    QDir(root.path()).mkdir(QStringLiteral(".partial-000009_x")); // an interrupted save
    const SnapshotStore store(root.path(), 1);
    ASSERT_TRUE(saveOne(store).ok());
    ASSERT_TRUE(saveOne(store).ok());

    EXPECT_TRUE(QDir(root.filePath(QStringLiteral("lost+found"))).exists());
    EXPECT_FALSE(QDir(root.filePath(QStringLiteral(".partial-000009_x"))).exists());
    EXPECT_EQ(store.list().size(), 1);
}

TEST_F(SnapshotStoreTest, UnwritableDirectoryReportsAnError)
{
    const SnapshotStore store(QStringLiteral("/proc/pivision-captures"), 0);
    const SaveResult result = saveOne(store);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.isEmpty());
}

} // namespace pivision::snapshot
