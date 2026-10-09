#ifndef PIVISION_DISPLAY_CAPTURECONTROL_H
#define PIVISION_DISPLAY_CAPTURECONTROL_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/Pipeline.h>
#include <pivision/snapshot/SnapshotStore.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

class MetricsMonitor;
class ProcessingControl;

// Backs the Capture button: saves the current frames, settings and metrics
// with a SnapshotStore. Exposed to QML as the singleton Captures through
// exposeCaptures().
class CaptureControl : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Captures)
    QML_SINGLETON
    Q_PROPERTY(QString directory READ directory CONSTANT)
    // True while a capture is being written; PNG encoding takes a moment on the Pi.
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    CaptureControl(pipeline::Pipeline *pipeline, MetricsMonitor *metrics,
        ProcessingControl *processing, snapshot::SnapshotStore store, QString appVersion,
        QObject *parent = nullptr);

    static CaptureControl *create(QQmlEngine *, QJSEngine *engine);
    inline static CaptureControl *instance_ = nullptr;

    QString directory() const { return store_.directory(); }
    bool busy() const { return busy_; }

    // Starts a capture of the newest frame. The result arrives as saved() or failed().
    Q_INVOKABLE void take();

signals:
    void busyChanged();
    void saved(const QString &name);
    void failed(const QString &message);

private:
    QJsonObject describe(const pipeline::DisplayFrame &frame) const;
    void setBusy(bool busy);

    QPointer<pipeline::Pipeline> pipeline_;
    QPointer<MetricsMonitor> metrics_;
    QPointer<ProcessingControl> processing_;
    snapshot::SnapshotStore store_;
    QString appVersion_;
    bool busy_ = false;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_CAPTURECONTROL_H
