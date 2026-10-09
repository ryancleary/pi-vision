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
    Q_PROPERTY(QString sourceName READ sourceName NOTIFY sourceNameChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
public:
    explicit Pipeline(std::unique_ptr<capture::FrameSource> source, QObject *parent = nullptr);
    ~Pipeline() override;

    void start();
    void stop();

    // Switches to `source`. The current feed stops immediately (cleared() is
    // emitted); if the new source fails to open, failed() reports why and no
    // frames arrive until another source is set.
    void setSource(std::unique_ptr<capture::FrameSource> source);

    // Call after frameAvailable(). Empty if the frame was already taken.
    std::optional<DisplayFrame> takeLatestFrame();

    std::uint64_t droppedFrames() const;
    QString sourceName() const;
    // Why the current source isn't producing frames; empty while it is.
    QString errorString() const;

signals:
    void frameAvailable();
    void failed(const QString &message);
    // Whatever is on screen is stale and should be removed.
    void cleared();
    void sourceNameChanged();
    void errorStringChanged();

private:
    void onOpened(const QString &name);
    void onFailed(const QString &message);
    void setSourceName(const QString &name);
    void setErrorString(const QString &message);

    // Order matters: members are destroyed bottom-up, so the thread is gone
    // before the producer, and the producer before the buffer it writes to.
    LatestFrameBuffer m_buffer;
    std::unique_ptr<FrameProducer> m_producer;
    QThread m_thread;
    QString m_sourceName;
    QString m_errorString;
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_PIPELINE_H
