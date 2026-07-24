#include "FrameProducer.h"

#include <utility>

#include <QImage>
#include <QTimer>
#include <QtMath>

namespace pivision::pipeline {

FrameProducer::FrameProducer(std::unique_ptr<capture::FrameSource> source,
    LatestFrameBuffer &buffer)
    : m_source(std::move(source))
    , m_buffer(buffer)
{
}

FrameProducer::~FrameProducer() = default;

void FrameProducer::start()
{
    if (!m_source->open()) {
        emit failed(QStringLiteral("Could not open %1").arg(QString::fromStdString(m_source->name())));
        return;
    }

    // Created here rather than in the constructor so the timer belongs to this thread.
    if (!m_timer) {
        m_timer = new QTimer(this);
        m_timer->setTimerType(Qt::PreciseTimer);
        connect(m_timer, &QTimer::timeout, this, &FrameProducer::tick);
    }
    const double fps = m_source->nominalFps() > 0.0 ? m_source->nominalFps() : 30.0;
    m_timer->start(qRound(1000.0 / fps));
}

void FrameProducer::stop()
{
    if (m_timer)
        m_timer->stop();
    m_source->close();
}

void FrameProducer::tick()
{
    if (!m_source->read(m_frame) || m_frame.image.type() != CV_8UC3) {
        stop();
        emit failed(QStringLiteral("%1 stopped producing frames")
                        .arg(QString::fromStdString(m_source->name())));
        return;
    }

    // Wrap the BGR pixels without copying, then deep-copy once so the QImage
    // owns its data before it leaves this thread.
    const cv::Mat &bgr = m_frame.image;
    const QImage view(bgr.data, bgr.cols, bgr.rows, static_cast<qsizetype>(bgr.step),
        QImage::Format_BGR888);

    if (m_buffer.put({ view.copy(), m_frame.index, m_frame.captured }))
        emit frameAvailable();
}

} // namespace pivision::pipeline

