#ifndef PIVISION_CAPTURE_CAMERADISCOVERY_H
#define PIVISION_CAPTURE_CAMERADISCOVERY_H

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

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_CAMERADISCOVERY_H
