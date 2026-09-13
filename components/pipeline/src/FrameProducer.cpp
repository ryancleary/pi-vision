#include "FrameProducer.h"

#include <chrono>
#include <utility>

#include <QTimer>

#include <pivision/logging/Logging.h>

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
    openSource();
}

void FrameProducer::stop()
{
    running_ = false;
    if (timer_)
        timer_->stop();
    if (source_)
        source_->close();
}

void FrameProducer::replaceSource(std::unique_ptr<capture::FrameSource> source)
{
    if (timer_)
        timer_->stop();
    if (source_)
        source_->close();

    source_ = std::move(source);
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
                             << framesRead_ << "frames";
        emit failed(QStringLiteral("%1 stopped producing frames").arg(nameOf(*source_)));
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

} // namespace pivision::pipeline
