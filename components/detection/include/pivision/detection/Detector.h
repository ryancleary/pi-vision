#ifndef PIVISION_DETECTION_DETECTOR_H
#define PIVISION_DETECTION_DETECTOR_H

#include <memory>
#include <vector>

#include <opencv2/core/mat.hpp>

#include <pivision/detection/Detection.h>
#include <pivision/detection/ModelConfig.h>

namespace pivision::detection {

// Finds objects in camera frames. The rest of the app only sees this; the
// model and how it runs stay inside the detection component.
class Detector {
public:
    virtual ~Detector() = default;

    // Objects in an 8-bit BGR frame, highest score first, boxes in the
    // frame's pixels.
    virtual std::vector<Detection> detect(const cv::Mat &bgr) const = 0;

    // The model's settings, including its class names (classId indexes them).
    virtual const ModelConfig &config() const = 0;
};

// Loads the model `config` describes. Throws cv::Exception if the model file
// can't be read.
std::unique_ptr<Detector> makeDetector(const ModelConfig &config);

} // namespace pivision::detection

#endif // PIVISION_DETECTION_DETECTOR_H
