#include <pivision/utils/Files.h>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>

namespace pivision::utils {

bool copyFileContents(const QString &from, const QString &to)
{
    QFile in(from);
    QFile out(to);
    if (!in.open(QIODevice::ReadOnly) || !out.open(QIODevice::WriteOnly))
        return false;
    constexpr qint64 kChunkBytes = 256 * 1024;
    while (!in.atEnd()) {
        const QByteArray chunk = in.read(kChunkBytes);
        if (chunk.isEmpty() || out.write(chunk) != chunk.size())
            return false;
    }
    return out.flush();
}

bool copyTree(const QString &from, const QString &to)
{
    const QDir source(from);
    if (!QDir().mkpath(to))
        return false;
    QDirIterator it(from, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QString target = QDir(to).filePath(source.relativeFilePath(path));
        const bool copied = it.fileInfo().isDir() ? QDir().mkpath(target) : copyFileContents(path, target);
        if (!copied)
            return false;
    }
    return true;
}

bool writeJsonFile(const QString &path, const QJsonObject &object)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    return file.commit();
}

} // namespace pivision::utils
