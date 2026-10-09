#ifndef PIVISION_DISPLAY_METRICSMONITOR_H
#define PIVISION_DISPLAY_METRICSMONITOR_H

#include <chrono>
#include <cstdint>
#include <optional>

#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/FrameMetrics.h>
#include <pivision/pipeline/Pipeline.h>
#include <pivision/pipeline/StageConfig.h>
#include <pivision/system/SystemProbe.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

// Backs the metrics panel: pipeline numbers averaged over the last second plus
// CPU, temperature and throttling, refreshed four times a second while
// `running`. Exposed to QML as the singleton MetricsMonitor through
// exposeMetricsMonitor().
class MetricsMonitor : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    // Polls only while true, so a hidden panel costs nothing.
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged)

    Q_PROPERTY(double captureFps READ captureFps NOTIFY updated)
    Q_PROPERTY(double displayFps READ displayFps NOTIFY updated)
    Q_PROPERTY(double latencyMs READ latencyMs NOTIFY updated)
    Q_PROPERTY(double stagesMs READ stagesMs NOTIFY updated)
    // Enabled stages in processing order: [ { label, ms } ]
    Q_PROPERTY(QVariantList stages READ stages NOTIFY updated)
    Q_PROPERTY(double droppedBeforeProcessing READ droppedBeforeProcessing NOTIFY updated)
    Q_PROPERTY(double droppedBeforeDisplay READ droppedBeforeDisplay NOTIFY updated)

    // Percent of one core; can exceed 100 on a multi-core CPU.
    Q_PROPERTY(double cpuPercent READ cpuPercent NOTIFY updated)
    Q_PROPERTY(bool hasTemperature READ hasTemperature NOTIFY updated)
    Q_PROPERTY(double temperature READ temperature NOTIFY updated)
    // False where the firmware doesn't report throttling (anything but a Pi).
    Q_PROPERTY(bool hasThrottle READ hasThrottle NOTIFY updated)
    Q_PROPERTY(QStringList throttleNow READ throttleNow NOTIFY updated)
    Q_PROPERTY(QStringList throttleSinceBoot READ throttleSinceBoot NOTIFY updated)

public:
    MetricsMonitor(pipeline::Pipeline *pipeline, const QList<pipeline::StageConfig> &stages,
        QObject *parent = nullptr);

    static MetricsMonitor *create(QQmlEngine *, QJSEngine *engine);
    inline static MetricsMonitor *instance_ = nullptr;

    bool running() const { return timer_.isActive(); }
    void setRunning(bool running);

    double captureFps() const { return frames_.captureFps; }
    double displayFps() const { return frames_.displayFps; }
    double latencyMs() const { return frames_.latencyMs; }
    double stagesMs() const { return frames_.stagesMs; }
    QVariantList stages() const;
    double droppedBeforeProcessing() const { return frames_.droppedBeforeProcessingPerSecond; }
    double droppedBeforeDisplay() const { return frames_.droppedBeforeDisplayPerSecond; }

    double cpuPercent() const { return cpuPercent_; }
    bool hasTemperature() const { return temperature_.has_value(); }
    double temperature() const { return temperature_.value_or(0.0); }
    bool hasThrottle() const { return throttleFlags_.has_value(); }
    QStringList throttleNow() const;
    QStringList throttleSinceBoot() const;

    // Takes a fresh reading and returns it all as JSON, for captures. If the
    // panel wasn't running, CPU use covers the time since the previous reading
    // (cpuWindowSeconds says how long).
    QJsonObject toJson();

signals:
    void runningChanged();
    void updated();

private:
    // Four refreshes a second: readable, and cheap for QML on the Pi.
    static constexpr std::chrono::milliseconds kRefreshInterval { 250 };

    void refresh();

    QPointer<pipeline::Pipeline> pipeline_;
    QHash<QString, QString> stageLabels_; // id -> label from stages.json
    QTimer timer_;
    system::CpuUsage cpu_;

    pipeline::MetricsSnapshot frames_;
    double cpuPercent_ = 0.0;
    std::optional<double> temperature_;
    std::optional<std::uint32_t> throttleFlags_;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_METRICSMONITOR_H
