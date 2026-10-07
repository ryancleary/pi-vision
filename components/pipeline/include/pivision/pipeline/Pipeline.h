#ifndef PIVISION_PIPELINE_PIPELINE_H
#define PIVISION_PIPELINE_PIPELINE_H

#include <cstdint>
#include <memory>
#include <optional>

#include <QObject>
#include <QString>
#include <QThread>

#include <pivision/capture/FrameSource.h>
#include <pivision/pipeline/LatestFrameBuffer.h>

namespace pivision::pipeline {

class FrameProducer;

// Runs a frame source on its own thread and hands the newest frame to the UI.
// Lives on the GUI thread.
class Pipeline : public QObject {
    Q_OBJECT
public:
    explicit Pipeline(std::unique_ptr<capture::FrameSource> source, QObject *parent = nullptr);
    ~Pipeline() override;

    void start();
    void stop();

    // Call after frameAvailable(). Empty if the frame was already taken.
    std::optional<DisplayFrame> takeLatestFrame();

    std::uint64_t droppedFrames() const;
    QString sourceName() const;

signals:
    void frameAvailable();
    void failed(const QString &message);

private:
    // Order matters: members are destroyed bottom-up, so the thread is gone
    // before the producer, and the producer before the buffer it writes to.
    LatestFrameBuffer m_buffer;
    std::unique_ptr<FrameProducer> m_producer;
    QThread m_thread;
    QString m_sourceName;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_PIPELINE_H

