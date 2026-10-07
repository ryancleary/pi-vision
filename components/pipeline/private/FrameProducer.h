#ifndef PIVISION_PIPELINE_FRAMEPRODUCER_H
#define PIVISION_PIPELINE_FRAMEPRODUCER_H

#include <memory>

#include <QObject>
#include <QString>

#include <pivision/capture/Frame.h>
#include <pivision/capture/FrameSource.h>
#include <pivision/pipeline/LatestFrameBuffer.h>

class QTimer;

namespace pivision::pipeline {

// Lives on the capture thread. Reads the source at its frame rate and
// puts each frame in the buffer.
class FrameProducer : public QObject {
    Q_OBJECT
public:
    FrameProducer(std::unique_ptr<capture::FrameSource> source, LatestFrameBuffer &buffer);
    ~FrameProducer() override;

public slots:
    void start();
    void stop();

signals:
    void frameAvailable();
    void failed(const QString &message);

private:
    void tick();

    std::unique_ptr<capture::FrameSource> m_source;
    LatestFrameBuffer &m_buffer;
    QTimer *m_timer = nullptr;
    capture::Frame m_frame; // reused every tick
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_FRAMEPRODUCER_H

