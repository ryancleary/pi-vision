#include <pivision/detection/Detector.h>

#include "ObjectDetector.h"

namespace pivision::detection {

std::unique_ptr<Detector> makeDetector(const ModelConfig &config)
{
    return std::make_unique<ObjectDetector>(config);
}

} // namespace pivision::detection
