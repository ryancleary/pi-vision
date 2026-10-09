#include <pivision/snapshot/Export.h>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QStorageInfo>

#include <pivision/snapshot/SnapshotStore.h>

namespace pivision::snapshot {

namespace {

    // Copies one file's bytes. Deliberately not QFile::copy or
    // std::filesystem::copy: those also copy permissions, which FAT sticks
    // refuse, and the contents are all that matter here.
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

    // Copies the folder `from` and everything in it to a new folder `to`.
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

} // namespace

ExportResult exportNumberedFolders(const QString &from, const QString &to)
{
    ExportResult result;
    const QDir source(from);
    if (!source.exists())
        return result;

    QDir destination(to);
    if (!destination.mkpath(QStringLiteral("."))) {
        result.error = QStringLiteral("Cannot create %1").arg(to);
        return result;
    }

    const QStringList names = source.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &name : names) {
        if (!isNumberedFolder(name))
            continue;
        if (destination.exists(name)) {
            ++result.skipped;
            continue;
        }

        const QString partial = destination.filePath(QStringLiteral(".partial-") + name);
        QDir(partial).removeRecursively(); // leftover from a stick pulled mid-copy
        if (!copyTree(source.filePath(name), partial) || !destination.rename(partial, name)) {
            QDir(partial).removeRecursively();
            result.error = QStringLiteral("Could not copy %1 to %2 (stick full or removed?)").arg(name, to);
            return result;
        }
        ++result.copied;
    }
    return result;
}

QStringList mountPointsUnder(const QString &root, const QStringList &mountPoints)
{
    const QString cleanRoot = QDir::cleanPath(root);
    QStringList below;
    for (const QString &mountPoint : mountPoints) {
        // Directly below: the parent folder is the root itself.
        const QString clean = QDir::cleanPath(mountPoint);
        if (clean != cleanRoot && QFileInfo(clean).path() == cleanRoot)
            below.append(clean);
    }
    below.sort();
    return below;
}

QStringList currentMountPoints()
{
    QStringList mountPoints;
    for (const QStorageInfo &volume : QStorageInfo::mountedVolumes()) {
        if (volume.isValid() && volume.isReady())
            mountPoints.append(volume.rootPath());
    }
    return mountPoints;
}

} // namespace pivision::snapshot
