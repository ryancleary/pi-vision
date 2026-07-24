#ifndef PIVISION_CAPTURE_SOURCEFACTORY_H
#define PIVISION_CAPTURE_SOURCEFACTORY_H

#include <memory>

#include <pivision/capture/FrameSource.h>

namespace pivision::capture {

struct TestPatternConfig {
    int width = 640;
    int height = 480;
    double fps = 30.0;
};

// Synthetic frames: a box moving across a gradient, with the frame index drawn on it.
// Needs no files or hardware, and produces identical output on every run.
std::unique_ptr<FrameSource> makeTestPatternSource(const TestPatternConfig &config = {});

} // namespace pivision::capture
#endif

