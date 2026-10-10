#include "Utils.h"

namespace pivision::snapshot {

const QRegularExpression &numberedFolderPattern()
{
    static const QRegularExpression pattern(QStringLiteral("^(\\d{6})_"));
    return pattern;
}

int sequenceOf(const QString &name)
{
    return numberedFolderPattern().match(name).captured(1).toInt();
}

QString partialFolderName(const QString &name)
{
    return QStringLiteral(".partial-") + name;
}

} // namespace pivision::snapshot
