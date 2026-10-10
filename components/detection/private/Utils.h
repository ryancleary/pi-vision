#ifndef PIVISION_DETECTION_UTILS_H
#define PIVISION_DETECTION_UTILS_H

#include <optional>
#include <string>
#include <vector>

#include <opencv2/core/persistence.hpp>

namespace pivision::detection {

// Model description settings look like { "value": ..., "about": ..., "source": [...] };
// these read the "value" of the setting at `node`. Each returns nothing if the
// setting is missing or has the wrong type.
std::optional<double> settingNumber(const cv::FileNode &node);
std::optional<std::string> settingString(const cv::FileNode &node);
std::optional<std::vector<double>> settingNumbers(const cv::FileNode &node);
std::optional<std::vector<std::string>> settingStrings(const cv::FileNode &node);

} // namespace pivision::detection

#endif // PIVISION_DETECTION_UTILS_H
