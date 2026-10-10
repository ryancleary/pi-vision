#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QTemporaryDir>

#include <pivision/utils/Files.h>

namespace pivision::utils {

class FilesTest : public ::testing::Test {
protected:
    // Writes `content` to `relative` below the temporary folder.
    void writeFile(const QString &relative, const QByteArray &content)
    {
        const QString path = root_.filePath(relative);
        QDir().mkpath(QFileInfo(path).path());
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(content);
    }

    QByteArray readFile(const QString &relative) const
    {
        QFile file(root_.filePath(relative));
        return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
    }

    QTemporaryDir root_;
};

TEST_F(FilesTest, GivenNestedFiles_WhenCopyingTheTree_ThenAllAreCopied)
{
    writeFile(QStringLiteral("from/a.txt"), "a");
    writeFile(QStringLiteral("from/sub/b.txt"), "b");
    ASSERT_TRUE(copyTree(root_.filePath(QStringLiteral("from")), root_.filePath(QStringLiteral("to"))));
    EXPECT_EQ(readFile(QStringLiteral("to/a.txt")), "a");
    EXPECT_EQ(readFile(QStringLiteral("to/sub/b.txt")), "b");
}

TEST_F(FilesTest, GivenAMissingSource_WhenCopyingContents_ThenItFails)
{
    EXPECT_FALSE(copyFileContents(root_.filePath(QStringLiteral("missing")), root_.filePath(QStringLiteral("out"))));
}

TEST_F(FilesTest, GivenAJsonObject_WhenWrittenAndReadBack_ThenItMatches)
{
    const QString path = root_.filePath(QStringLiteral("info.json"));
    ASSERT_TRUE(writeJsonFile(path, QJsonObject { { QStringLiteral("answer"), 42 } }));
    EXPECT_EQ(QJsonDocument::fromJson(readFile(QStringLiteral("info.json"))).object().value(QStringLiteral("answer")).toInt(), 42);
}

} // namespace pivision::utils
