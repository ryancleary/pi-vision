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

// Lives on the capture thread. Reads the source at its frame rate and puts
// each frame in the raw buffer for the processing thread.
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

private:
    void openSource();
    void tick();

    std::unique_ptr<capture::FrameSource> source_;
    RawFrameBuffer &buffer_;
    QTimer *timer_ = nullptr;
    capture::Frame frame_;
    // Counts source replacements; frames carry it so stale ones can be dropped.
    std::uint64_t generation_ = 0;
    std::uint64_t framesRead_ = 0;
    bool running_ = false;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_FRAMEPRODUCER_H
