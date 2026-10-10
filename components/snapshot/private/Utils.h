#ifndef PIVISION_SNAPSHOT_UTILS_H
#define PIVISION_SNAPSHOT_UTILS_H

#include <QRegularExpression>
#include <QString>

namespace pivision::snapshot {

// "000012_...": a six-digit sequence number and an underscore, then anything.
// Captures and crash dumps are both named this way.
const QRegularExpression &numberedFolderPattern();

// The sequence number at the start of a numbered folder's name.
int sequenceOf(const QString &name);

// Name a folder is written under until it's complete, then renamed from:
// ".partial-<name>". partialFolderName("*") gives the glob for leftovers.
QString partialFolderName(const QString &name);

} // namespace pivision::snapshot

#endif // PIVISION_SNAPSHOT_UTILS_H
