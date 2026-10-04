#include "ScenarioPlayer.h"

#include <cstdlib>
#include <utility>

#include <QCoreApplication>
#include <QJSEngine>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include <pivision/logging/Logging.h>
#include <pivision/utils/Files.h>

#include "CaptureControl.h"
#include "DetectionControl.h"
#include "ProcessingControl.h"
#include "Utils.h"

namespace pivision::display {

ScenarioPlayer::ScenarioPlayer(pipeline::Pipeline *pipeline, ProcessingControl *processing,
    DetectionControl *detection, CaptureControl *captures, QObject *parent)
    : QObject(parent)
    , pipeline_(pipeline)
    , processing_(processing)
    , detection_(detection)
    , captures_(captures)
    , timer_(new QTimer(this))
{
    timer_->setSingleShot(true);
}

ScenarioPlayer *ScenarioPlayer::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

void ScenarioPlayer::play(Scenario scenario, QString name, QString resultsPath)
{
    scenario_ = std::move(scenario);
    name_ = std::move(name);
    resultsPath_ = std::move(resultsPath);
    if (!pipeline_ || scenario_.steps.isEmpty())
        return;

    qCInfo(logging::lcApp) << "Scenario" << name_ << "with" << scenario_.steps.size()
                           << "steps; waiting for the first frame";
    // Start on the first frame, so every run begins from the same state.
    auto started = std::make_shared<QMetaObject::Connection>();
    *started = connect(pipeline_, &pipeline::Pipeline::frameAvailable, this, [this, started] {
        disconnect(*started);
        timer_->stop();
        clock_.start();
        runStep();
    });
    connect(timer_, &QTimer::timeout, this, [this, started] {
        if (!clock_.isValid()) {
            disconnect(*started);
            qCCritical(logging::lcApp) << "Scenario: no frame within" << kFirstFrameTimeoutMs << "ms";
            ++failures_;
            finish();
        }
    }, Qt::SingleShotConnection);
    timer_->start(kFirstFrameTimeoutMs);
}

void ScenarioPlayer::runStep()
{
    if (current_ >= scenario_.steps.size()) {
        finish();
        return;
    }
    const ScenarioStep &step = scenario_.steps.at(current_);
    qCInfo(logging::lcApp).noquote() << QStringLiteral("Scenario step %1/%2:").arg(current_ + 1)
                                            .arg(scenario_.steps.size())
                                     << step.text;
    stepClock_.start();

    switch (step.action) {
    case ScenarioStep::Action::Mode:
        emit modeRequested(step.value);
        finishStep(true, QString());
        return;

    case ScenarioStep::Action::Stages: {
        if (!processing_) {
            finishStep(false, QStringLiteral("no processing control"));
            return;
        }
        QStringList unknown = step.stages;
        for (const pipeline::StageConfig &stage : processing_->stageConfigs()) {
            unknown.removeAll(stage.id);
            processing_->setEnabled(stage.id, step.stages.contains(stage.id));
        }
        if (!unknown.isEmpty())
            finishStep(false, QStringLiteral("unknown stage(s): %1").arg(unknown.join(QStringLiteral(", "))));
        else
            finishStep(true, QString());
        return;
    }

    case ScenarioStep::Action::DetectionInput:
        if (detection_)
            detection_->setInput(step.value == QStringLiteral("processed") ? DetectionControl::Processed
                                                                           : DetectionControl::Raw);
        finishStep(detection_ != nullptr, detection_ ? QString() : QStringLiteral("no detection control"));
        return;

    case ScenarioStep::Action::Wait:
        stepConnections_.append(connect(timer_, &QTimer::timeout, this, [this] { finishStep(true, QString()); }));
        timer_->start(static_cast<int>(step.seconds * 1000.0));
        return;

    case ScenarioStep::Action::Capture:
        if (!captures_) {
            finishStep(false, QStringLiteral("no capture control"));
            return;
        }
        stepConnections_.append(connect(captures_, &CaptureControl::saved, this,
            [this](const QString &name) { finishStep(true, QString(), name); }));
        stepConnections_.append(connect(captures_, &CaptureControl::failed, this,
            [this](const QString &message) { finishStep(false, message); }));
        waitFor(kCaptureTimeoutMs, QStringLiteral("capture not written in time"));
        captures_->take(step.label);
        return;

    case ScenarioStep::Action::ExpectFrames:
        framesSeen_ = 0;
        stepConnections_.append(connect(pipeline_, &pipeline::Pipeline::frameAvailable, this, [this, step] {
            if (++framesSeen_ >= step.atLeast)
                finishStep(true, QStringLiteral("%1 frames").arg(framesSeen_));
        }));
        waitFor(static_cast<int>(step.seconds * 1000.0),
            QStringLiteral("only %1 of %2 frames").arg(framesSeen_).arg(step.atLeast));
        return;

    case ScenarioStep::Action::ExpectDetection:
        stepConnections_.append(connect(pipeline_, &pipeline::Pipeline::detectionAvailable, this, [this, step] {
            const auto result = pipeline_->latestDetection();
            if (!result)
                return;
            for (const detection::Detection &found : result->detections) {
                if (classLabel(*result, found.classId) == step.label) {
                    finishStep(true, QStringLiteral("%1 at score %2").arg(step.label).arg(found.score, 0, 'f', 2));
                    return;
                }
            }
        }));
        // A detector that can't load its model will never find anything; say so now.
        stepConnections_.append(connect(pipeline_, &pipeline::Pipeline::detectorStateChanged, this, [this] {
            if (pipeline_->detectorState() == pipeline::DetectorState::Failed)
                finishStep(false, QStringLiteral("detector failed: %1").arg(pipeline_->detectorError()));
        }));
        waitFor(static_cast<int>(step.seconds * 1000.0), QStringLiteral("no %1 detected").arg(step.label));
        if (pipeline_->detectorState() == pipeline::DetectorState::Failed)
            finishStep(false, QStringLiteral("detector failed: %1").arg(pipeline_->detectorError()));
        return;
    }
}

void ScenarioPlayer::waitFor(int timeoutMs, const QString &timeoutDetail)
{
    // The detail is fixed when the step starts, except for the frame count,
    // which is filled in when the time runs out.
    stepConnections_.append(connect(timer_, &QTimer::timeout, this, [this, timeoutDetail] {
        const ScenarioStep &step = scenario_.steps.at(current_);
        finishStep(false, step.action == ScenarioStep::Action::ExpectFrames
                ? QStringLiteral("only %1 of %2 frames").arg(framesSeen_).arg(step.atLeast)
                : timeoutDetail);
    }));
    timer_->start(timeoutMs);
}

void ScenarioPlayer::finishStep(bool passed, const QString &detail, const QString &capture)
{
    timer_->stop();
    for (const QMetaObject::Connection &connection : std::as_const(stepConnections_))
        disconnect(connection);
    stepConnections_.clear();

    const ScenarioStep &step = scenario_.steps.at(current_);
    if (!passed) {
        ++failures_;
        qCWarning(logging::lcApp).noquote() << QStringLiteral("Scenario step %1 FAILED:").arg(current_ + 1)
                                            << detail;
    }
    QJsonObject result {
        { QStringLiteral("step"), static_cast<int>(current_ + 1) },
        { QStringLiteral("do"), QJsonDocument::fromJson(step.text.toUtf8()).object() },
        { QStringLiteral("passed"), passed },
        { QStringLiteral("startedMs"), clock_.elapsed() - stepClock_.elapsed() },
        { QStringLiteral("tookMs"), stepClock_.elapsed() },
    };
    if (!detail.isEmpty())
        result.insert(QStringLiteral("detail"), detail);
    if (!capture.isEmpty())
        result.insert(QStringLiteral("capture"), capture);
    results_.append(result);

    ++current_;
    // Queued, so a step finishing inside a signal handler doesn't start the
    // next one from within it.
    QMetaObject::invokeMethod(this, &ScenarioPlayer::runStep, Qt::QueuedConnection);
}

void ScenarioPlayer::finish()
{
    const bool passed = failures_ == 0;
    qCInfo(logging::lcApp) << "Scenario" << name_ << (passed ? "passed" : "FAILED") << "-"
                           << failures_ << "failed check(s) in" << results_.size() << "step(s)";
    if (!resultsPath_.isEmpty()) {
        const QJsonObject results {
            { QStringLiteral("scenario"), name_ },
            { QStringLiteral("about"), scenario_.about },
            { QStringLiteral("source"), pipeline_ ? pipeline_->sourceName() : QString() },
            { QStringLiteral("passed"), passed },
            { QStringLiteral("failures"), failures_ },
            { QStringLiteral("steps"), results_ },
        };
        if (!utils::writeJsonFile(resultsPath_, results))
            qCWarning(logging::lcApp) << "Could not write" << resultsPath_;
    }
    QCoreApplication::exit(passed ? EXIT_SUCCESS : EXIT_FAILURE);
}

} // namespace pivision::display
