#include <pivision/capture/CameraDiscovery.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <system_error>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace pivision::capture {

namespace {

    // Number after "video" in a device name, or -1 if the name doesn't match.
    int deviceNumber(const std::string &filename)
    {
        constexpr const char prefix[] = "video";
        if (filename.rfind(prefix, 0) != 0 || filename.size() == sizeof(prefix) - 1)
            return -1;
        const std::string digits = filename.substr(sizeof(prefix) - 1);
        if (!std::all_of(digits.begin(), digits.end(),
                [](unsigned char c) { return std::isdigit(c) != 0; }))
            return -1;
        return std::atoi(digits.c_str());
    }

    // Asks the driver what the node is. Returns false for anything that isn't a
    // V4L2 video capture node (metadata nodes, output devices, plain files).
    bool queryCapture(const std::string &path, std::string &name)
    {
        const int fd = ::open(path.c_str(), O_RDWR | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0)
            return false;

        v4l2_capability caps {};
        const bool queried = ::ioctl(fd, VIDIOC_QUERYCAP, &caps) == 0;
        ::close(fd);
        if (!queried)
            return false;

        // device_caps describes this node; capabilities describes the whole device.
        const std::uint32_t nodeCaps
            = (caps.capabilities & V4L2_CAP_DEVICE_CAPS) ? caps.device_caps : caps.capabilities;
        if (!(nodeCaps & V4L2_CAP_VIDEO_CAPTURE))
            return false;

        name.assign(reinterpret_cast<const char *>(caps.card),
            strnlen(reinterpret_cast<const char *>(caps.card), sizeof(caps.card)));
        return true;
    }

} // namespace

std::vector<CameraInfo> findCameras(const std::string &directory)
{
    std::vector<std::pair<int, CameraInfo>> found;

    std::error_code error;
    for (const auto &entry : std::filesystem::directory_iterator(directory, error)) {
        const int number = deviceNumber(entry.path().filename().string());
        if (number < 0)
            continue;

        CameraInfo info;
        info.device = entry.path().string();
        if (queryCapture(info.device, info.name))
            found.emplace_back(number, std::move(info));
    }

    // video2 before video10: sort by number, not by name.
    std::sort(found.begin(), found.end(),
        [](const auto &a, const auto &b) { return a.first < b.first; });

    std::vector<CameraInfo> cameras;
    cameras.reserve(found.size());
    for (auto &item : found)
        cameras.push_back(std::move(item.second));
    return cameras;
}

} // namespace pivision::capture
