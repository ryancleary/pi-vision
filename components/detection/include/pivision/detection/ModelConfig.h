#ifndef PIVISION_DETECTION_MODELCONFIG_H
#define PIVISION_DETECTION_MODELCONFIG_H

#include <optional>
#include <string>
#include <vector>

#include <opencv2/core/types.hpp>

namespace pivision::detection {

// Color channel order a model expects. Our frames are always BGR.
enum class ChannelOrder { Bgr, Rgb };

// How a frame is fitted to the model's input size.
enum class ResizeMode {
    Letterbox, // keep the aspect ratio, pad the rest with zeros
    Stretch,   // resize to exactly the input size
};

// Everything model-specific, read from a model description file such as
// assets/models/nanodet-plus-m-1.5x-416.json. That file explains each value
// and links to where it comes from; this struct only carries the values.
struct ModelConfig {
    std::string name;
    std::string modelPath; // the ONNX file, resolved next to the description

    // Input
    cv::Size inputSize;
    ChannelOrder channelOrder = ChannelOrder::Bgr;
    ResizeMode resize = ResizeMode::Letterbox;
    cv::Scalar mean;              // per channel, in the model's channel order
    cv::Scalar standardDeviation; // per channel, in the model's channel order

    // Output
    std::vector<int> strides;
    int regMax = 0;
    float scoreThreshold = 0.0F;
    float nmsThreshold = 0.0F;
    std::vector<std::string> classes;
};

// Reads a model description. Returns nothing, with the reason in `*error`,
// if the file can't be read or a setting is missing or invalid.
std::optional<ModelConfig> loadModelConfig(const std::string &path, std::string *error = nullptr);

} // namespace pivision::detection

#endif // PIVISION_DETECTION_MODELCONFIG_H

