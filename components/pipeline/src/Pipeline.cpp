#include <pivision/pipeline/Pipeline.h>

#include <utility>

#include "FrameProducer.h"

namespace pivision::pipeline {

Pipeline::Pipeline(std::unique_ptr<capture::FrameSource> source, QObject *parent)
    : QObject(parent)
    , m_sourceName(QString::fromStdString(source->name()))
{
    m_producer = std::make_unique<FrameProducer>(std::move(source), m_buffer);
    m_producer->moveToThread(&m_thread);
    m_thread.setObjectName(QStringLiteral("capture"));

    connect(&m_thread, &QThread::started, m_producer.get(), &FrameProducer::start);
    // Producer -> Pipeline crosses threads, so these are queued automatically.
    connect(m_producer.get(), &FrameProducer::frameAvailable, this, &Pipeline::frameAvailable);
    connect(m_producer.get(), &FrameProducer::failed, this, &Pipeline::failed);
}

Pipeline::~Pipeline()
{
    stop();
}

void Pipeline::start()
{
    if (!m_thread.isRunning())
        m_thread.start();
}

void Pipeline::stop()
{
    if (!m_thread.isRunning())
        return;

    // Run stop() on the producer's own thread and wait for it, so the timer and
    // source are shut down by the thread that owns them.
    QMetaObject::invokeMethod(m_producer.get(), &FrameProducer::stop, Qt::BlockingQueuedConnection);
    m_thread.quit();
    m_thread.wait();
}

std::optional<DisplayFrame> Pipeline::takeLatestFrame()
{
    return m_buffer.take();
}

std::uint64_t Pipeline::droppedFrames() const
{
    return m_buffer.droppedCount();
}

QString Pipeline::sourceName() const
{
    return m_sourceName;
}

} // namespace pivision::pipeline

