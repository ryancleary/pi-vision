#include "Utils.h"

#include <regex>

namespace pivision::capture {

std::optional<int> deviceNumber(const std::string &filename)
{
    static const std::regex pattern("^video([0-9]+)$");
    std::smatch match;
    if (!std::regex_match(filename, match, pattern))
        return std::nullopt;
    return std::stoi(match[1].str());
}

} // namespace pivision::capture
