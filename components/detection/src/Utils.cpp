#include "Utils.h"

#include <numeric>

#include <opencv2/core.hpp>

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

cv::Mat findOutput(const std::vector<cv::Mat> &outputs, int rows, int columns)
{
    for (const cv::Mat &output : outputs) {
        if (output.total() == static_cast<size_t>(rows) * static_cast<size_t>(columns)
            && output.size[output.dims - 1] == columns)
            return output.reshape(1, rows);
    }
    return {};
}

float expectedValue(const cv::Mat &logits)
{
    // Softmax: p_i = exp(x_i) / sum of exp(x_j).
    //     https://pytorch.org/docs/stable/generated/torch.nn.functional.softmax.html
    cv::Mat probabilities;
    cv::exp(logits, probabilities);
    probabilities /= cv::sum(probabilities)[0];

    // The values 0, 1, ..., n-1 the probabilities belong to.
    cv::Mat values(logits.size(), CV_32F);
    std::iota(values.begin<float>(), values.end<float>(), 0.0F);

    return static_cast<float>(probabilities.dot(values));
}

} // namespace pivision::detection
