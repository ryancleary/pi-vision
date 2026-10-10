#ifndef PIVISION_PIPELINE_DETECTIONRESULT_H
#define PIVISION_PIPELINE_DETECTIONRESULT_H

#include <chrono>
#include <cstdint>
#include <vector>

#include <QImage>
#include <QMetaType>

#include <pivision/detection/Detection.h>

namespace pivision::pipeline {

// Which image the detector looks at.
enum class DetectionInput {
    Raw,       // the camera frame as captured
    Processed, // the output of the processing stages
};

// What the detector is doing; the UI shows it in Detection mode.
enum class DetectorState {
    Off,     // detection not enabled, or no model loaded yet
    Loading, // reading the model (the first time detection is enabled)
    Ready,   // running
    Failed,  // the model couldn't be loaded; see Pipeline::detectorError()
};

// One run of the detector.
struct DetectionResult {
    std::vector<detection::Detection> detections; // boxes in `image`'s pixels
    QImage image;                                 // the image the boxes are on
    DetectionInput input = DetectionInput::Raw;
    std::uint64_t frameIndex = 0;                 // which capture it came from
    std::chrono::steady_clock::time_point captured; // when that capture happened
    double milliseconds = 0.0;                    // how long detect() took
    std::uint64_t generation = 0;                 // see DisplayFrame
};

} // namespace pivision::pipeline

// Lets DetectorState travel in queued signals between threads.
Q_DECLARE_METATYPE(pivision::pipeline::DetectorState)

#endif // PIVISION_PIPELINE_DETECTIONRESULT_H
