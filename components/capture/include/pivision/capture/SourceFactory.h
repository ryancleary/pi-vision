#ifndef PIVISION_CAPTURE_SOURCEFACTORY_H
#define PIVISION_CAPTURE_SOURCEFACTORY_H

#include <memory>
#include <string>
#include <vector>

#include <pivision/capture/FrameSource.h>

namespace pivision::capture {

struct TestPatternConfig {
    int width = 640;
    int height = 480;
    double fps = 30.0;
};

// What to request from a camera. Cameras pick the nearest mode they support,
// so the frames that arrive may differ; check Frame::image for the real size.
struct CameraConfig {
    int width = 640;
    int height = 480;
    double fps = 30.0;
    // MJPEG keeps USB bandwidth low; nearly every UVC webcam supports it.
    bool mjpeg = true;
};

// Synthetic frames: a box moving across a gradient, with the frame index drawn on it.
// Needs no files or hardware, and produces identical output on every run.
std::unique_ptr<FrameSource> makeTestPatternSource(const TestPatternConfig &config = {});

// A V4L2 camera, such as "/dev/video0".
std::unique_ptr<FrameSource> makeCameraSource(const std::string &device,
    const CameraConfig &config = {});

// A video file or stream URL, opened by whichever OpenCV backend supports it.
// Files loop at the end; streams don't.
std::unique_ptr<FrameSource> makeVideoSource(const std::string &location);

// Picks the source type from a string: "/dev/video*" is a camera, anything else
// is a file or stream. Used for the command line and the UI's "Other..." entry.
std::unique_ptr<FrameSource> makeSourceFromSpec(const std::string &spec,
    const CameraConfig &config = {});

// Uses the first of `devices` that opens, otherwise the test pattern.
// The choice is made in open(), on whichever thread runs the source.
std::unique_ptr<FrameSource> makeAutoSource(std::vector<std::string> devices,
    const CameraConfig &config = {}, const TestPatternConfig &fallback = {});

// makeAutoSource() over the cameras findCameras() reports.
std::unique_ptr<FrameSource> makeAutoSource(const CameraConfig &config = {},
    const TestPatternConfig &fallback = {});

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_SOURCEFACTORY_H
