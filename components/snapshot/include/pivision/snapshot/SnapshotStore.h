#ifndef PIVISION_SNAPSHOT_SNAPSHOTSTORE_H
#define PIVISION_SNAPSHOT_SNAPSHOTSTORE_H

#include <utility>

#include <QImage>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace pivision::snapshot {

struct SaveResult {
    QString name;  // folder name, e.g. 000012_2026-10-09_143501
    QString path;  // full path of the folder
    QString error; // why it failed; empty on success

    bool ok() const { return error.isEmpty(); }
};

// Saves the app's state at a moment (the UI calls these "captures") into
// numbered folders under one directory:
//
//   <directory>/000012_2026-10-09_143501/
//       info.json       settings and metrics, as given, plus sequence and time
//       raw.png         the camera frame
//       processed.png   the processed frame
//
// The number orders captures even when the clock is wrong (the Pi has no
// real-time clock). Only the newest `keep` are kept; 0 keeps them all.
class SnapshotStore {
public:
    SnapshotStore(QString directory, int keep)
        : directory_(std::move(directory))
        , keep_(keep)
    {
    }

    const QString &directory() const { return directory_; }
    int keep() const { return keep_; }

    // Writes a new capture, then deletes the oldest beyond `keep`. The folder
    // appears complete or not at all: it's written under a temporary name
    // and renamed at the end, so a copy taken meanwhile never sees half of it.
    SaveResult save(const QImage &raw, const QImage &processed, QJsonObject info) const;

    // Folder names of the saved captures, oldest first.
    QStringList list() const;

private:
    QString directory_;
    int keep_;
};

} // namespace pivision::snapshot

#endif // PIVISION_SNAPSHOT_SNAPSHOTSTORE_H
