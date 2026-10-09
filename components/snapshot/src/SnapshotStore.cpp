#include <pivision/snapshot/SnapshotStore.h>

#include <algorithm>

#include <QDateTime>
#include <QDir>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>

#include <unistd.h>

#include <pivision/logging/Logging.h>

namespace pivision::snapshot {

namespace {

    // "000012_..." : a six-digit sequence number, then anything.
    const QRegularExpression &numberedName()
    {
        static const QRegularExpression pattern(QStringLiteral("^(\\d{6})_"));
        return pattern;
    }

    const QString kPartialPrefix = QStringLiteral(".partial-");

    // The sequence number at the start of a capture's folder name.
    int sequenceOf(const QString &name)
    {
        return numberedName().match(name).captured(1).toInt();
    }

    bool writeJson(const QString &path, const QJsonObject &object)
    {
        // QSaveFile writes to a temporary file and renames it on commit().
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly))
            return false;
        file.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
        return file.commit();
    }

} // namespace

bool isNumberedFolder(const QString &name)
{
    return numberedName().match(name).hasMatch();
}

QStringList SnapshotStore::list() const
{
    QStringList names = QDir(directory_).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    names.erase(std::remove_if(names.begin(), names.end(),
                    [](const QString &name) { return !isNumberedFolder(name); }),
        names.end());
    // The fixed-width number at the front makes name order the same as age order.
    names.sort();
    return names;
}

SaveResult SnapshotStore::save(const QImage &raw, const QImage &processed, QJsonObject info) const
{
    SaveResult result;
    QDir dir(directory_);
    if (!dir.mkpath(QStringLiteral("."))) {
        result.error = QStringLiteral("Cannot create %1").arg(directory_);
        return result;
    }

    // Leftovers from a save that was cut short (power loss, crash).
    for (const QString &partial : dir.entryList({ kPartialPrefix + QLatin1Char('*') },
             QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot))
        QDir(dir.filePath(partial)).removeRecursively();

    const QStringList existing = list();
    const int sequence = existing.isEmpty() ? 1 : sequenceOf(existing.last()) + 1;
    const QDateTime now = QDateTime::currentDateTime();
    result.name = QStringLiteral("%1_%2")
                      .arg(sequence, 6, 10, QLatin1Char('0'))
                      .arg(now.toString(QStringLiteral("yyyy-MM-dd_HHmmss")));
    result.path = dir.filePath(result.name);

    const QString partialPath = dir.filePath(kPartialPrefix + result.name);
    QDir partial(partialPath);
    info.insert(QStringLiteral("sequence"), sequence);
    info.insert(QStringLiteral("time"), now.toString(Qt::ISODate));

    const bool written = dir.mkpath(partialPath)
        && raw.save(partial.filePath(QStringLiteral("raw.png")))
        && processed.save(partial.filePath(QStringLiteral("processed.png")))
        && writeJson(partial.filePath(QStringLiteral("info.json")), info)
        && dir.rename(partialPath, result.path);
    if (!written) {
        partial.removeRecursively();
        result.error = QStringLiteral("Could not write %1 (disk full or read-only?)").arg(result.path);
        return result;
    }

    // Oldest first, so drop from the front until `keep` remain.
    if (keep_ > 0) {
        const QStringList all = list();
        for (qsizetype i = 0; i < all.size() - keep_; ++i)
            QDir(dir.filePath(all.at(i))).removeRecursively();
    }

    // On the Pi this is an SD card that may lose power at any moment.
    ::sync();
    qCInfo(logging::lcApp) << "Saved capture" << result.path;
    return result;
}

} // namespace pivision::snapshot
