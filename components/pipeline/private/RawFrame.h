#ifndef PIVISION_PIPELINE_RAWFRAME_H
#define PIVISION_PIPELINE_RAWFRAME_H

#include <cstdint>

#include <pivision/capture/Frame.h>
#include <pivision/pipeline/LatestFrameBuffer.h>

namespace pivision::pipeline {

// A captured frame on its way from the capture thread to the processing thread.
struct RawFrame {
    capture::Frame frame;
    std::uint64_t generation = 0;
};

using RawFrameBuffer = LatestBuffer<RawFrame>;

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_RAWFRAME_H
