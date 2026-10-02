#ifndef PIVISION_PIPELINE_FRAMEPRODUCER_H
#define PIVISION_PIPELINE_FRAMEPRODUCER_H

#include <cstdint>
#include <memory>

#include <QObject>
#include <QString>

#include <pivision/capture/Frame.h>
#include <pivision/capture/FrameSource.h>

#include "RawFrame.h"

class QTimer;

namespace pivision::pipeline {

class CameraWatcher;

// Lives on the capture thread. Reads the source at its frame rate and puts
// each frame in the raw buffer for the processing thread.
//
// If a source that was working stops (a webcam unplugged, or reset by USB),
// the producer waits for it: whenever udev reports a camera appearing, it
// asks the source to reopen(), and carries on once that works.
class FrameProducer : public QObject {
    Q_OBJECT
public:
    FrameProducer(std::unique_ptr<capture::FrameSource> source, RawFrameBuffer &buffer);
    ~FrameProducer() override;

    // Closes the current source and switches to `source`, opening it if the
    // producer is running. Call on the capture thread, or before it starts.
    void replaceSource(std::unique_ptr<capture::FrameSource> source);

public slots:
    void start();
    void stop();

signals:
    void opened(const QString &name);
    void frameAvailable();
    void failed(const QString &message);
    // A camera was plugged in or removed; the list of sources has changed.
    void camerasChanged();

private:
    void openSource();
    void startReading();
    void tick();
    void onCameraAdded();
    void tryReopen();

    std::unique_ptr<capture::FrameSource> source_;
    RawFrameBuffer &buffer_;
    QTimer *timer_ = nullptr;
    CameraWatcher *watcher_ = nullptr; // created in start(), on the capture thread
    capture::Frame frame_;
    // Counts source replacements; frames carry it so stale ones can be dropped.
    std::uint64_t generation_ = 0;
    std::uint64_t framesRead_ = 0;
    bool running_ = false;
    // The source stopped producing frames; reopen it when a camera appears.
    bool waitingForSource_ = false;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_FRAMEPRODUCER_H
