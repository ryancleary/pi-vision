#ifndef PIVISION_DETECTION_UTILS_H
#define PIVISION_DETECTION_UTILS_H

#include <optional>
#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/core/persistence.hpp>

namespace pivision::detection {

// Model description settings look like { "value": ..., "about": ..., "source": [...] };
// these read the "value" of the setting at `node`. Each returns nothing if the
// setting is missing or has the wrong type.
std::optional<double> settingNumber(const cv::FileNode &node);
std::optional<std::string> settingString(const cv::FileNode &node);
std::optional<std::vector<double>> settingNumbers(const cv::FileNode &node);
std::optional<std::vector<std::string>> settingStrings(const cv::FileNode &node);

// Finds the output with `rows` x `columns` values (for one image) and returns
// it as a rows x columns matrix sharing its data, or an empty Mat if there is
// none. Model outputs come as 1 x rows x columns; matching by shape avoids
// depending on the order the model lists its outputs in.
cv::Mat findOutput(const std::vector<cv::Mat> &outputs, int rows, int columns);

// The expected value of a probability distribution over 0, 1, ..., n-1 that
// is given as n raw scores ("logits") in a 1 x n row: softmax turns the
// scores into probabilities p_i, and the result is the sum of p_i * i.
float expectedValue(const cv::Mat &logits);

} // namespace pivision::detection

#endif // PIVISION_DETECTION_UTILS_H
