#include <pivision/snapshot/Export.h>

#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>

#include <pivision/snapshot/SnapshotStore.h>
#include <pivision/utils/Files.h>

#include "Utils.h"

namespace pivision::snapshot {

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

        const QString partial = destination.filePath(partialFolderName(name));
        QDir(partial).removeRecursively(); // leftover from a stick pulled mid-copy
        if (!utils::copyTree(source.filePath(name), partial) || !destination.rename(partial, name)) {
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
