#include "MetricsMonitor.h"

#include <chrono>

#include <QJSEngine>
#include <QJsonArray>
#include <QVariantMap>

#include <pivision/logging/Logging.h>
#include <pivision/utils/Strings.h>

namespace pivision::display {

MetricsMonitor::MetricsMonitor(
    pipeline::Pipeline *pipeline, const QList<pipeline::StageConfig> &stages, QObject *parent)
    : QObject(parent)
    , pipeline_(pipeline)
{
    for (const pipeline::StageConfig &stage : stages)
        stageLabels_.insert(stage.id, stage.label);
    timer_.setInterval(kRefreshInterval);
    connect(&timer_, &QTimer::timeout, this, &MetricsMonitor::refresh);
}

MetricsMonitor *MetricsMonitor::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

void MetricsMonitor::setRunning(bool running)
{
    if (running == timer_.isActive())
        return;
    if (running) {
        cpu_.sample(); // start the CPU interval now, not when the panel was last shown
        timer_.start();
        refresh();
    } else {
        timer_.stop();
    }
    qCDebug(logging::lcDisplay) << "Metrics" << (running ? "running" : "stopped");
    emit runningChanged();
}

QVariantList MetricsMonitor::stages() const
{
    QVariantList list;
    for (const processing::StageTiming &timing : frames_.stages) {
        const QString id = QString::fromStdString(timing.id);
        list.append(QVariantMap {
            { QStringLiteral("label"), stageLabels_.value(id, id) },
            { QStringLiteral("ms"), timing.milliseconds },
        });
    }
    return list;
}

QStringList MetricsMonitor::throttleNow() const
{
    return throttleFlags_ ? utils::toQStringList(system::activeThrottleConditions(*throttleFlags_))
                          : QStringList();
}

QStringList MetricsMonitor::throttleSinceBoot() const
{
    return throttleFlags_ ? utils::toQStringList(system::pastThrottleConditions(*throttleFlags_))
                          : QStringList();
}

QJsonObject MetricsMonitor::toJson()
{
    refresh();

    QJsonArray stageTimes;
    for (const processing::StageTiming &timing : frames_.stages) {
        stageTimes.append(QJsonObject {
            { QStringLiteral("id"), QString::fromStdString(timing.id) },
            { QStringLiteral("ms"), timing.milliseconds },
        });
    }

    QJsonObject json {
        { QStringLiteral("captureFps"), frames_.captureFps },
        { QStringLiteral("displayFps"), frames_.displayFps },
        { QStringLiteral("latencyMs"), frames_.latencyMs },
        { QStringLiteral("stagesMs"), frames_.stagesMs },
        { QStringLiteral("stages"), stageTimes },
        { QStringLiteral("droppedBeforeProcessingPerSecond"), frames_.droppedBeforeProcessingPerSecond },
        { QStringLiteral("droppedBeforeDisplayPerSecond"), frames_.droppedBeforeDisplayPerSecond },
        { QStringLiteral("cpuPercent"), cpuPercent_ },
        { QStringLiteral("cpuWindowSeconds"), cpu_.lastInterval().count() },
        // null where there's no sensor or it isn't a Pi
        { QStringLiteral("temperatureC"), temperature_ ? QJsonValue(*temperature_) : QJsonValue() },
    };
    if (throttleFlags_) {
        json.insert(QStringLiteral("throttle"), QJsonObject {
            { QStringLiteral("flags"), QStringLiteral("0x%1").arg(*throttleFlags_, 0, 16) },
            { QStringLiteral("now"), QJsonArray::fromStringList(throttleNow()) },
            { QStringLiteral("sinceBoot"), QJsonArray::fromStringList(throttleSinceBoot()) },
        });
    } else {
        json.insert(QStringLiteral("throttle"), QJsonValue());
    }
    return json;
}

void MetricsMonitor::refresh()
{
    frames_ = pipeline_ ? pipeline_->metrics() : pipeline::MetricsSnapshot {};
    cpuPercent_ = cpu_.sample();
    temperature_ = system::readTemperatureCelsius();
    throttleFlags_ = system::readThrottleFlags();
    emit updated();
}

} // namespace pivision::display
