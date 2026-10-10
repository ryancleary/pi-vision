#include "Utils.h"

namespace pivision::detection {

std::optional<double> settingNumber(const cv::FileNode &node)
{
    const cv::FileNode value = node["value"];
    if (!value.isInt() && !value.isReal())
        return std::nullopt;
    return value.real();
}

std::optional<std::string> settingString(const cv::FileNode &node)
{
    const cv::FileNode value = node["value"];
    if (!value.isString())
        return std::nullopt;
    return value.string();
}

std::optional<std::vector<double>> settingNumbers(const cv::FileNode &node)
{
    const cv::FileNode value = node["value"];
    if (!value.isSeq() || value.empty())
        return std::nullopt;
    std::vector<double> numbers;
    for (const cv::FileNode &item : value) {
        if (!item.isInt() && !item.isReal())
            return std::nullopt;
        numbers.push_back(item.real());
    }
    return numbers;
}

std::optional<std::vector<std::string>> settingStrings(const cv::FileNode &node)
{
    const cv::FileNode value = node["value"];
    if (!value.isSeq() || value.empty())
        return std::nullopt;
    std::vector<std::string> strings;
    for (const cv::FileNode &item : value) {
        if (!item.isString())
            return std::nullopt;
        strings.push_back(item.string());
    }
    return strings;
}

} // namespace pivision::detection
