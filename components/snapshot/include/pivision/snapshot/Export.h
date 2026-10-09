#ifndef PIVISION_SNAPSHOT_EXPORT_H
#define PIVISION_SNAPSHOT_EXPORT_H

#include <QString>
#include <QStringList>

namespace pivision::snapshot {

struct ExportResult {
    int copied = 0;  // folders copied this time
    int skipped = 0; // already at the destination
    QString error;   // why it stopped; empty on success

    bool ok() const { return error.isEmpty(); }
};

// Copies each numbered folder (captures, crash dumps) in `from` that `to`
// doesn't have yet, so repeating an export only adds what's new. Each folder
// is copied under a temporary name and renamed when complete. A missing
// `from` is not an error: there's simply nothing to copy.
ExportResult exportNumberedFolders(const QString &from, const QString &to);

// The entries of `mountPoints` that sit directly below `root`, sorted:
// with root /run/media, "/run/media/sda1" is one, "/run/media" and
// "/run/media/sda1/x" are not.
QStringList mountPointsUnder(const QString &root, const QStringList &mountPoints);

// Every mounted filesystem's mount point, from the system's mount table.
QStringList currentMountPoints();

} // namespace pivision::snapshot

#endif // PIVISION_SNAPSHOT_EXPORT_H
