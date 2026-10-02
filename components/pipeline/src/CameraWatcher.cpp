#include "CameraWatcher.h"

#include <QSocketNotifier>

#include <libudev.h>

#include <pivision/logging/Logging.h>

namespace pivision::pipeline {

CameraWatcher::CameraWatcher(QObject *parent)
    : QObject(parent)
    , udev_(udev_new())
{
    if (udev_)
        monitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    // V4L2 devices are udev's "video4linux" subsystem.
    if (!monitor_ || udev_monitor_filter_add_match_subsystem_devtype(monitor_, "video4linux", nullptr) < 0
        || udev_monitor_enable_receiving(monitor_) < 0) {
        qCWarning(logging::lcCapture) << "Cannot watch for cameras (no udev); a camera that "
                                         "drops out won't be picked up again by itself";
        return;
    }
    notifier_ = new QSocketNotifier(udev_monitor_get_fd(monitor_), QSocketNotifier::Read, this);
    connect(notifier_, &QSocketNotifier::activated, this, &CameraWatcher::onReadable);
}

CameraWatcher::~CameraWatcher()
{
    delete notifier_;
    if (monitor_)
        udev_monitor_unref(monitor_);
    if (udev_)
        udev_unref(udev_);
}

void CameraWatcher::onReadable()
{
    udev_device *device = udev_monitor_receive_device(monitor_);
    if (!device)
        return;
    const QString action = QString::fromLatin1(udev_device_get_action(device));
    const char *node = udev_device_get_devnode(device);
    if (node) {
        const QString path = QString::fromLocal8Bit(node);
        qCDebug(logging::lcCapture) << "udev:" << action << path;
        if (action == QStringLiteral("add"))
            emit cameraAdded(path);
        else if (action == QStringLiteral("remove"))
            emit cameraRemoved(path);
    }
    udev_device_unref(device);
}

} // namespace pivision::pipeline
