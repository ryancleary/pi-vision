#ifndef PIVISION_PIPELINE_DETECTIONJOB_H
#define PIVISION_PIPELINE_DETECTIONJOB_H

#include <chrono>
#include <cstdint>

#include <QImage>

#include <opencv2/core/mat.hpp>

#include <pivision/pipeline/DetectionResult.h>
#include <pivision/pipeline/LatestFrameBuffer.h>

namespace pivision::pipeline {

// A frame handed from the processing thread to the detection thread.
struct DetectionJob {
    cv::Mat bgr;   // what the detector reads
    QImage image;  // the same picture, for showing with the result
    DetectionInput input = DetectionInput::Raw;
    std::uint64_t frameIndex = 0;
    std::chrono::steady_clock::time_point captured;
    std::uint64_t generation = 0;
};

using DetectionJobBuffer = LatestBuffer<DetectionJob>;
using DetectionResultBuffer = LatestBuffer<DetectionResult>;

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_DETECTIONJOB_H
