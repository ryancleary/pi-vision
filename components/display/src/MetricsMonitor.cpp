#include "MetricsMonitor.h"

#include <chrono>

#include <QJSEngine>
#include <QVariantMap>

#include <pivision/logging/Logging.h>

namespace pivision::display {

namespace {

    // Four refreshes a second: readable, and cheap for QML on the Pi.
    constexpr std::chrono::milliseconds kRefreshInterval(250);

    QStringList toQStringList(const std::vector<std::string> &labels)
    {
        QStringList list;
        for (const std::string &label : labels)
            list.append(QString::fromStdString(label));
        return list;
    }

} // namespace

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
    return throttleFlags_ ? toQStringList(system::activeThrottleConditions(*throttleFlags_))
                          : QStringList();
}

QStringList MetricsMonitor::throttleSinceBoot() const
{
    return throttleFlags_ ? toQStringList(system::pastThrottleConditions(*throttleFlags_))
                          : QStringList();
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
