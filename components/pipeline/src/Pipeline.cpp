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
    connect(m_producer.get(), &FrameProducer::opened, this, &Pipeline::onOpened);
    connect(m_producer.get(), &FrameProducer::failed, this, &Pipeline::onFailed);
}

Pipeline::~Pipeline() { stop(); }

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

void Pipeline::setSource(std::unique_ptr<capture::FrameSource> source)
{
    if (!source)
        return;

    // Cut the old feed right away; the new one appears once it opens.
    setSourceName(QString::fromStdString(source->name()));
    setErrorString({});
    emit cleared();

    if (!m_thread.isRunning()) {
        // Nothing runs on the capture thread yet, so it's safe to call directly.
        m_producer->replaceSource(std::move(source));
        return;
    }

    // Opening can be slow (cameras), so it happens on the capture thread. The
    // shared holder frees the source even if the call never runs, for example
    // when the pipeline stops before the queued call is processed.
    auto holder = std::make_shared<std::unique_ptr<capture::FrameSource>>(std::move(source));
    FrameProducer *producer = m_producer.get();
    QMetaObject::invokeMethod(
        producer, [producer, holder] { producer->replaceSource(std::move(*holder)); },
        Qt::QueuedConnection);
}

std::optional<DisplayFrame> Pipeline::takeLatestFrame() { return m_buffer.take(); }

std::uint64_t Pipeline::droppedFrames() const { return m_buffer.droppedCount(); }

QString Pipeline::sourceName() const { return m_sourceName; }

QString Pipeline::errorString() const { return m_errorString; }

void Pipeline::onOpened(const QString &name)
{
    setSourceName(name);
    setErrorString({});
}

void Pipeline::onFailed(const QString &message)
{
    setErrorString(message);
    emit failed(message);
}

void Pipeline::setSourceName(const QString &name)
{
    if (name == m_sourceName)
        return;
    m_sourceName = name;
    emit sourceNameChanged();
}

void Pipeline::setErrorString(const QString &message)
{
    if (message == m_errorString)
        return;
    m_errorString = message;
    emit errorStringChanged();
}

} // namespace pivision::pipeline
