#include "FrameProducer.h"

#include <chrono>
#include <utility>

#include <QTimer>

#include <pivision/logging/Logging.h>

#include "CameraWatcher.h"
#include "Utils.h"

namespace pivision::pipeline {

FrameProducer::FrameProducer(
    std::unique_ptr<capture::FrameSource> source, RawFrameBuffer &buffer)
    : source_(std::move(source))
    , buffer_(buffer)
{
}

FrameProducer::~FrameProducer() = default;

void FrameProducer::start()
{
    running_ = true;
    if (!watcher_) {
        watcher_ = new CameraWatcher(this);
        connect(watcher_, &CameraWatcher::cameraAdded, this, &FrameProducer::onCameraAdded);
        connect(watcher_, &CameraWatcher::cameraAdded, this, &FrameProducer::camerasChanged);
        connect(watcher_, &CameraWatcher::cameraRemoved, this, &FrameProducer::camerasChanged);
    }
    openSource();
}

void FrameProducer::stop()
{
    running_ = false;
    waitingForSource_ = false;
    if (timer_)
        timer_->stop();
    if (source_)
        source_->close();
    // Its socket notifier must go on this thread, not when the producer is
    // destroyed on another one.
    delete watcher_;
    watcher_ = nullptr;
}

void FrameProducer::replaceSource(std::unique_ptr<capture::FrameSource> source)
{
    if (timer_)
        timer_->stop();
    if (source_)
        source_->close();

    source_ = std::move(source);
    waitingForSource_ = false;
    ++generation_;
    buffer_.take(); // drop any frame from the old source

    if (running_)
        openSource();
}

void FrameProducer::openSource()
{
    if (!source_)
        return;

    qCDebug(logging::lcCapture) << "Opening" << nameOf(*source_);
    if (!source_->open()) {
        qCWarning(logging::lcCapture) << "Could not open" << nameOf(*source_);
        emit failed(QStringLiteral("Could not open %1").arg(nameOf(*source_)));
        return;
    }
    startReading();
}

void FrameProducer::startReading()
{
    // Created here rather than in the constructor so the timer belongs to this thread.
    if (!timer_) {
        timer_ = new QTimer(this);
        timer_->setTimerType(Qt::PreciseTimer);
        connect(timer_, &QTimer::timeout, this, &FrameProducer::tick);
    }

    // The name may only be known once open, e.g. which camera auto picked.
    emit opened(nameOf(*source_));
    const double fps = source_->nominalFps() > 0.0 ? source_->nominalFps() : 30.0;
    qCInfo(logging::lcCapture, "Opened %s at %.1f fps", qPrintable(nameOf(*source_)), fps);
    framesRead_ = 0;
    // Read once per frame period: 1/fps seconds, as whole milliseconds for QTimer.
    timer_->start(std::chrono::round<std::chrono::milliseconds>(std::chrono::duration<double>(1.0 / fps)));
}

void FrameProducer::tick()
{
    if (!source_->read(frame_) || frame_.image.type() != CV_8UC3) {
        // Stop reading but stay running, so a replacement source opens normally.
        timer_->stop();
        source_->close();
        qCWarning(logging::lcCapture) << nameOf(*source_) << "stopped producing frames after"
                             << framesRead_ << "frames; waiting for it to come back";
        emit failed(QStringLiteral("%1 stopped producing frames. Waiting for it to come back…")
                        .arg(nameOf(*source_)));
        waitingForSource_ = true;
        // It may already be back: a webcam reset by USB returns within seconds,
        // possibly before the failed read noticed it was gone.
        tryReopen();
        return;
    }

    // Hand the frame over without copying pixels. Moving it out leaves frame_
    // empty, so the next read allocates fresh memory instead of overwriting
    // pixels the processing thread may still be using.
    ++framesRead_;
    if (buffer_.put({ std::move(frame_), generation_ }))
        emit frameAvailable();
    frame_ = {};
}

void FrameProducer::onCameraAdded()
{
    if (waitingForSource_)
        tryReopen();
}

void FrameProducer::tryReopen()
{
    if (!running_ || !waitingForSource_ || !source_)
        return;
    if (!source_->reopen()) {
        qCDebug(logging::lcCapture) << nameOf(*source_) << "not back yet";
        return;
    }
    waitingForSource_ = false;
    qCInfo(logging::lcCapture) << nameOf(*source_) << "is back";
    startReading();
}

} // namespace pivision::pipeline
