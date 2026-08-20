#ifndef PIVISION_CAPTURE_CAMERADISCOVERY_H
#define PIVISION_CAPTURE_CAMERADISCOVERY_H

#include <optional>
#include <string>
#include <vector>

namespace pivision::capture {

struct CameraInfo {
    std::string device; // "/dev/video0"
    std::string name;   // as the driver reports it, e.g. "HD Webcam C270"
};

// V4L2 devices in `directory` that can capture video, in device-number order.
// A UVC webcam usually creates two nodes, one for video and one for metadata;
// only the video node is returned.
std::vector<CameraInfo> findCameras(const std::string &directory = "/dev");

// Driver details for one device node, or nothing if it isn't a video capture node.
std::optional<CameraInfo> describeCamera(const std::string &device);

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_CAMERADISCOVERY_H
