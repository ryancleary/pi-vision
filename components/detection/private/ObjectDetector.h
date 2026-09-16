#ifndef PIVISION_DETECTION_OBJECTDETECTOR_H
#define PIVISION_DETECTION_OBJECTDETECTOR_H

#include <string>
#include <vector>

#include <opencv2/core/mat.hpp>
#include <opencv2/dnn.hpp>

#include <pivision/detection/Detection.h>
#include <pivision/detection/ModelConfig.h>

namespace pivision::detection {

// Finds objects in a camera frame with a neural network, run by OpenCV DNN:
// loads the model, prepares each frame as the model's input, runs it, turns
// the output into boxes, and maps the boxes back onto the frame.
//
// Everything specific to the model (input size, normalization, strides,
// classes...) comes from its ModelConfig, i.e. its description file in
// assets/models/, which documents each value and its source. Turning the
// output into boxes follows the NanoDet-Plus format; a model family with a
// different output format would need its own decoding.
// OpenCV DNN API used here (Image2BlobParams, blobFromImageWithParams):
//     https://docs.opencv.org/4.9.0/d9/d3c/structcv_1_1dnn_1_1Image2BlobParams.html
class ObjectDetector {
public:
    // Loads the model file named in `config`. Throws cv::Exception if it
    // can't be read.
    explicit ObjectDetector(ModelConfig config);

    const ModelConfig &config() const { return config_; }

    // Turns an 8-bit BGR frame into the model's input: fitted to the input
    // size, normalized per channel, laid out as a 1 x 3 x height x width
    // float blob.
    cv::Mat makeInputBlob(const cv::Mat &bgr) const;

    // Runs the model on an input blob from makeInputBlob(). Returns its raw
    // outputs, one Mat per output layer, in outputNames() order.
    std::vector<cv::Mat> run(const cv::Mat &blob) const;

    // Turns the raw outputs into candidate detections, in the pixels of the
    // model's input: one per grid cell whose best class scores at least the
    // model's scoreThreshold. Neighboring cells often find the same object,
    // so duplicates remain; removing them (NMS) is a separate step.
    std::vector<Detection> decode(const std::vector<cv::Mat> &outputs) const;

    // Maps a box in the model's input back onto the original frame, undoing
    // the resize (and the padding, for letterbox).
    cv::Rect toImageRect(const cv::Rect &inputRect, const cv::Size &imageSize) const;

    // Names of the model's output layers, in the order forward() returns them.
    std::vector<std::string> outputNames() const { return net_.getUnconnectedOutLayersNames(); }

private:
    // Turns the config's input settings into OpenCV's preprocessing parameters.
    static cv::dnn::Image2BlobParams blobParams(const ModelConfig &config);

    ModelConfig config_;
    mutable cv::dnn::Net net_; // forward() isn't const in OpenCV
    cv::dnn::Image2BlobParams params_;
};

} // namespace pivision::detection

#endif // PIVISION_DETECTION_OBJECTDETECTOR_H
