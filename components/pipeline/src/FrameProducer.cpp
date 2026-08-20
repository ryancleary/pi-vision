#include "FrameProducer.h"

#include <utility>

#include <QImage>
#include <QTimer>
#include <QtMath>

namespace pivision::pipeline {

namespace {

    QString nameOf(const capture::FrameSource &source)
    {
        return QString::fromStdString(source.name());
    }

} // namespace

FrameProducer::FrameProducer(
    std::unique_ptr<capture::FrameSource> source, LatestFrameBuffer &buffer)
    : m_source(std::move(source))
    , m_buffer(buffer)
{
}

FrameProducer::~FrameProducer() = default;

void FrameProducer::start()
{
    m_running = true;
    openSource();
}

void FrameProducer::stop()
{
    m_running = false;
    if (m_timer)
        m_timer->stop();
    if (m_source)
        m_source->close();
}

void FrameProducer::replaceSource(std::unique_ptr<capture::FrameSource> source)
{
    if (m_timer)
        m_timer->stop();
    if (m_source)
        m_source->close();

    m_source = std::move(source);
    m_buffer.take(); // drop any frame from the old source

    if (m_running)
        openSource();
}

void FrameProducer::openSource()
{
    if (!m_source)
        return;

    if (!m_source->open()) {
        emit failed(QStringLiteral("Could not open %1").arg(nameOf(*m_source)));
        return;
    }

    // Created here rather than in the constructor so the timer belongs to this thread.
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(m_timer, &QTimer::timeout, this, &FrameProducer::tick);
    }

    // The name may only be known once open, e.g. which camera auto picked.
    emit opened(nameOf(*m_source));
    const double fps = m_source->nominalFps() > 0.0 ? m_source->nominalFps() : 30.0;
    m_timer->start(qRound(1000.0 / fps));
}

void FrameProducer::tick()
{
    if (!m_source->read(m_frame) || m_frame.image.type() != CV_8UC3) {
        // Stop reading but stay running, so a replacement source opens normally.
        m_timer->stop();
        m_source->close();
        emit failed(QStringLiteral("%1 stopped producing frames").arg(nameOf(*m_source)));
        return;
    }

    // Wrap the BGR pixels without copying, then deep-copy once so the QImage
    // owns its data before it leaves this thread.
    const cv::Mat &bgr = m_frame.image;
    const QImage view(
        bgr.data, bgr.cols, bgr.rows, static_cast<qsizetype>(bgr.step), QImage::Format_BGR888);

    if (m_buffer.put({ view.copy(), m_frame.index, m_frame.captured }))
        emit frameAvailable();
}

} // namespace pivision::pipeline
