#ifndef PIVISION_UTILS_FILES_H
#define PIVISION_UTILS_FILES_H

#include <QJsonObject>
#include <QString>

namespace pivision::utils {

// Copies one file's bytes. Deliberately not QFile::copy or
// std::filesystem::copy: those also copy permissions, which FAT USB sticks
// refuse, and the contents are all that matter here.
bool copyFileContents(const QString &from, const QString &to);

// Copies the folder `from` and everything in it to a new folder `to`, file
// contents only (see copyFileContents).
bool copyTree(const QString &from, const QString &to);

// Writes `object` as indented JSON. The file is replaced only once fully
// written (QSaveFile), so a crash never leaves half a file.
bool writeJsonFile(const QString &path, const QJsonObject &object);

} // namespace pivision::utils

#endif // PIVISION_UTILS_FILES_H
