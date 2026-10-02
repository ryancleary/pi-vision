#ifndef PIVISION_PIPELINE_CAMERAWATCHER_H
#define PIVISION_PIPELINE_CAMERAWATCHER_H

#include <QObject>
#include <QString>

class QSocketNotifier;
struct udev;
struct udev_monitor;

namespace pivision::pipeline {

// Reports cameras (V4L2 device nodes) appearing and disappearing, as udev
// announces them, so nothing has to poll. udev's monitor is a socket: a
// QSocketNotifier turns "an event is waiting" into a Qt signal.
//
// Listens for "udev" events rather than raw "kernel" ones: udev sends them
// after its rules have run, so the device's permissions are already set.
// https://www.freedesktop.org/software/systemd/man/255/udev_monitor_new_from_netlink.html
//
// Belongs to the thread that creates it (its socket notifier does), and must
// be destroyed there.
class CameraWatcher : public QObject {
    Q_OBJECT
public:
    explicit CameraWatcher(QObject *parent = nullptr);
    ~CameraWatcher() override;

    // False if udev couldn't be reached; no signals arrive then.
    bool isWatching() const { return notifier_ != nullptr; }

signals:
    void cameraAdded(const QString &device);   // e.g. "/dev/video0"
    void cameraRemoved(const QString &device);

private:
    void onReadable();

    udev *udev_ = nullptr;
    udev_monitor *monitor_ = nullptr;
    QSocketNotifier *notifier_ = nullptr;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_CAMERAWATCHER_H
