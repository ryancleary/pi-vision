#include "NanoDetDetector.h"

#include <utility>

namespace pivision::detection {

NanoDetDetector::NanoDetDetector(ModelConfig config)
    : config_(std::move(config))
    , net_(cv::dnn::readNetFromONNX(config_.modelPath))
    , params_(blobParams(config_))
{
}

cv::dnn::Image2BlobParams NanoDetDetector::blobParams(const ModelConfig &config)
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

cv::Mat NanoDetDetector::makeInputBlob(const cv::Mat &bgr) const
{
    return cv::dnn::blobFromImageWithParams(bgr, params_);
}

cv::Rect NanoDetDetector::toImageRect(const cv::Rect &inputRect, const cv::Size &imageSize) const
{
    // Image2BlobParams knows how it resized, so it can undo it:
    //     https://github.com/opencv/opencv/blob/4.9.0/modules/dnn/src/dnn_utils.cpp#L413-L439
    // blobRectToImageRect isn't const in OpenCV 4.9, hence the copy.
    cv::dnn::Image2BlobParams params = params_;
    return params.blobRectToImageRect(inputRect, imageSize);
}

} // namespace pivision::detection

