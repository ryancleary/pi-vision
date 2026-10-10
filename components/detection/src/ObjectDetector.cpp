#include "ObjectDetector.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <opencv2/core.hpp>

#include "Utils.h"

namespace pivision::detection {

ObjectDetector::ObjectDetector(ModelConfig config)
    : config_(std::move(config))
    , net_(cv::dnn::readNetFromONNX(config_.modelPath))
    , params_(blobParams(config_))
{
}

cv::dnn::Image2BlobParams ObjectDetector::blobParams(const ModelConfig &config)
{
    // Models are trained on (pixel - mean) / std. OpenCV computes
    // (pixel - mean) * scalefactor, so scalefactor is 1 / std per channel:
    //     https://github.com/opencv/opencv/blob/4.9.0/modules/dnn/include/opencv2/dnn/dnn.hpp#L1206
    const cv::Scalar scale(1.0 / config.standardDeviation[0], 1.0 / config.standardDeviation[1],
        1.0 / config.standardDeviation[2]);

    // Our frames are BGR; swap to RGB only if the model wants RGB.
    const bool swapRB = config.channelOrder == ChannelOrder::Rgb;

    // Letterbox: shrink keeping the aspect ratio, pad the rest with zeros,
    // centered. NULL: plain resize to the input size. What OpenCV does for each:
    //     https://github.com/opencv/opencv/blob/4.9.0/modules/dnn/src/dnn_utils.cpp#L187-L204
    const cv::dnn::ImagePaddingMode padding = config.resize == ResizeMode::Letterbox
        ? cv::dnn::DNN_PMODE_LETTERBOX
        : cv::dnn::DNN_PMODE_NULL;

    return cv::dnn::Image2BlobParams(scale, config.inputSize, config.mean, swapRB, CV_32F,
        cv::dnn::DNN_LAYOUT_NCHW, padding);
}

cv::Mat ObjectDetector::makeInputBlob(const cv::Mat &bgr) const
{
    return cv::dnn::blobFromImageWithParams(bgr, params_);
}

std::vector<cv::Mat> ObjectDetector::run(const cv::Mat &blob) const
{
    std::vector<cv::Mat> outputs;
    net_.setInput(blob);
    net_.forward(outputs, outputNames());
    return outputs;
}

// How NanoDet-Plus encodes its output, from its own decoding code:
//     https://github.com/RangiLyu/nanodet/blob/be9b4a9001d7f9b6fc89c2df31ae8d428e35b4f0/nanodet/model/head/nanodet_plus_head.py#L464-L497
// Per stride (scale), the input is split into a grid of stride x stride pixel
// cells, and the model outputs two tables with one row per cell:
//   scores: one per class, already probabilities (0..1); the ONNX export
//           applies a sigmoid (nanodet_plus_head.py#L552)
//   boxes:  4 x (regMax + 1) values: for the left, top, right and bottom
//           edge, scores over the distances 0..regMax cells from the cell
std::vector<Detection> ObjectDetector::decode(const std::vector<cv::Mat> &outputs) const
{
    const int classCount = static_cast<int>(config_.classes.size());
    const int binCount = config_.regMax + 1; // distances 0..regMax
    constexpr int kEdges = 4;                // left, top, right, bottom
    const cv::Rect2f inputArea(0.0F, 0.0F, static_cast<float>(config_.inputSize.width),
        static_cast<float>(config_.inputSize.height));

    std::vector<Detection> candidates;
    for (const int stride : config_.strides) {
        // Cells across and down; NanoDet rounds a partial cell up (#L480).
        const cv::Size grid(cvCeil(static_cast<double>(config_.inputSize.width) / stride),
            cvCeil(static_cast<double>(config_.inputSize.height) / stride));
        const cv::Mat scores = findOutput(outputs, grid.area(), classCount);
        const cv::Mat boxes = findOutput(outputs, grid.area(), kEdges * binCount);
        if (scores.empty() || boxes.empty())
            throw std::runtime_error("model has no outputs for stride " + std::to_string(stride));

        for (int cell = 0; cell < grid.area(); ++cell) {
            // The most likely class for this cell, and how likely it is.
            double score = 0.0;
            cv::Point best;
            cv::minMaxLoc(scores.row(cell), nullptr, &score, nullptr, &best);
            if (score < config_.scoreThreshold)
                continue;

            // Each edge's distance is the expected value of its distribution,
            // in cells; times the stride gives pixels (Integral, gfl_head.py):
            //     https://github.com/RangiLyu/nanodet/blob/be9b4a9001d7f9b6fc89c2df31ae8d428e35b4f0/nanodet/model/head/gfl_head.py#L36-L67
            const cv::Mat edges = boxes.row(cell).reshape(1, kEdges); // one row per edge
            const float left = expectedValue(edges.row(0)) * static_cast<float>(stride);
            const float top = expectedValue(edges.row(1)) * static_cast<float>(stride);
            const float right = expectedValue(edges.row(2)) * static_cast<float>(stride);
            const float bottom = expectedValue(edges.row(3)) * static_cast<float>(stride);

            // Distances are measured from the cell's top-left corner, in input
            // pixels (get_single_level_center_priors, #L515-L536). The Model
            // Zoo's decoder shifts this point by (stride - 1) / 2 (nanodet.py
            // #L30-L31); we follow NanoDet's own code. The grid is stored row
            // by row, so cell index = row * width + column.
            const cv::Point2f origin(static_cast<float>((cell % grid.width) * stride),
                static_cast<float>((cell / grid.width) * stride));

            // Box from the four distances (distance2bbox), kept inside the input:
            //     https://github.com/RangiLyu/nanodet/blob/be9b4a9001d7f9b6fc89c2df31ae8d428e35b4f0/nanodet/util/box_transform.py#L4-L25
            const cv::Rect2f box = cv::Rect2f(cv::Point2f(origin.x - left, origin.y - top),
                                       cv::Point2f(origin.x + right, origin.y + bottom))
                & inputArea;

            candidates.push_back(Detection { cv::Rect(box), best.x, static_cast<float>(score) });
        }
    }
    return candidates;
}

cv::Rect ObjectDetector::toImageRect(const cv::Rect &inputRect, const cv::Size &imageSize) const
{
    // Image2BlobParams knows how it resized, so it can undo it:
    //     https://github.com/opencv/opencv/blob/4.9.0/modules/dnn/src/dnn_utils.cpp#L413-L439
    // blobRectToImageRect isn't const in OpenCV 4.9, hence the copy.
    cv::dnn::Image2BlobParams params = params_;
    return params.blobRectToImageRect(inputRect, imageSize);
}

} // namespace pivision::detection
