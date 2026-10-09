#ifndef PIVISION_UTILS_STRINGS_H
#define PIVISION_UTILS_STRINGS_H

#include <string>
#include <vector>

#include <QStringList>

namespace pivision::utils {

QStringList toQStringList(const std::vector<std::string> &strings);

} // namespace pivision::utils

#endif // PIVISION_UTILS_STRINGS_H
