#include <pivision/utils/Strings.h>

namespace pivision::utils {

QStringList toQStringList(const std::vector<std::string> &strings)
{
    QStringList list;
    list.reserve(static_cast<qsizetype>(strings.size()));
    for (const std::string &string : strings)
        list.append(QString::fromStdString(string));
    return list;
}

} // namespace pivision::utils
