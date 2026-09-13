#include <pivision/detection/ModelConfig.h>

#include <filesystem>

#include <opencv2/core.hpp>

#include "Utils.h"

namespace pivision::detection {

std::optional<ModelConfig> loadModelConfig(const std::string &path, std::string *error)
{
    const auto fail = [error](const std::string &message) -> std::optional<ModelConfig> {
        if (error)
            *error = message;
        return std::nullopt;
    };

    // OpenCV's FileStorage reads JSON (and YAML/XML); it throws on a syntax error.
    cv::FileStorage file;
    try {
        if (!file.open(path, cv::FileStorage::READ | cv::FileStorage::FORMAT_JSON))
            return fail(path + ": cannot open");
    } catch (const cv::Exception &exception) {
        return fail(path + ": not valid JSON (" + exception.msg + ")");
    }
    const cv::FileNode root = file.root();
    const cv::FileNode input = root["input"];
    const cv::FileNode output = root["output"];

    ModelConfig config;
    config.name = root["name"].string();

    const auto modelFile = settingString(root["modelFile"]);
    if (!modelFile)
        return fail(path + ": modelFile needs a string value");
    // The model sits in the same folder as its description.
    config.modelPath = (std::filesystem::path(path).parent_path() / *modelFile).string();

    const auto size = settingNumbers(input["size"]);
    if (!size || size->size() != 2)
        return fail(path + ": input.size needs [width, height]");
    config.inputSize = cv::Size(static_cast<int>((*size)[0]), static_cast<int>((*size)[1]));

    const auto channelOrder = settingString(input["channelOrder"]);
    if (channelOrder == std::string("BGR"))
        config.channelOrder = ChannelOrder::Bgr;
    else if (channelOrder == std::string("RGB"))
        config.channelOrder = ChannelOrder::Rgb;
    else
        return fail(path + ": input.channelOrder must be \"BGR\" or \"RGB\"");

    const auto resize = settingString(input["resize"]);
    if (resize == std::string("letterbox"))
        config.resize = ResizeMode::Letterbox;
    else if (resize == std::string("stretch"))
        config.resize = ResizeMode::Stretch;
    else
        return fail(path + ": input.resize must be \"letterbox\" or \"stretch\"");

    const auto mean = settingNumbers(input["mean"]);
    const auto standardDeviation = settingNumbers(input["std"]);
    if (!mean || mean->size() != 3 || !standardDeviation || standardDeviation->size() != 3)
        return fail(path + ": input.mean and input.std need three numbers each");
    config.mean = cv::Scalar((*mean)[0], (*mean)[1], (*mean)[2]);
    config.standardDeviation
        = cv::Scalar((*standardDeviation)[0], (*standardDeviation)[1], (*standardDeviation)[2]);

    const auto strides = settingNumbers(output["strides"]);
    if (!strides)
        return fail(path + ": output.strides needs a list of numbers");
    for (const double stride : *strides)
        config.strides.push_back(static_cast<int>(stride));

    const auto regMax = settingNumber(output["regMax"]);
    const auto scoreThreshold = settingNumber(output["scoreThreshold"]);
    const auto nmsThreshold = settingNumber(output["nmsThreshold"]);
    if (!regMax || !scoreThreshold || !nmsThreshold)
        return fail(path + ": output needs regMax, scoreThreshold and nmsThreshold");
    config.regMax = static_cast<int>(*regMax);
    config.scoreThreshold = static_cast<float>(*scoreThreshold);
    config.nmsThreshold = static_cast<float>(*nmsThreshold);

    const auto classes = settingStrings(root["classes"]);
    if (!classes)
        return fail(path + ": classes needs a list of names");
    config.classes = *classes;

    return config;
}

} // namespace pivision::detection
