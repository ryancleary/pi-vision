#ifndef PIVISION_DISPLAY_SCENARIOPLAYER_H
#define PIVISION_DISPLAY_SCENARIOPLAYER_H

#include <QElapsedTimer>
#include <QJsonArray>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QtQml/qqmlregistration.h>

#include <pivision/display/Scenario.h>
#include <pivision/pipeline/Pipeline.h>

class QQmlEngine;
class QJSEngine;
class QTimer;

namespace pivision::display {

class CaptureControl;
class DetectionControl;
class ProcessingControl;

// Plays a Scenario (see Scenario.h) against the running app: one step at a
// time, through the same objects the UI uses, so it tests what a person would
// see. Starts once the first frame is on screen. When the last step is done
// it writes the results file and quits the app: exit code 0 if every check
// passed, 1 if not.
//
// Exposed to QML as the singleton Scenario through exposeScenario(); QML
// owns the view mode, so "mode" steps arrive there as modeRequested().
// Without a scenario it does nothing.
class ScenarioPlayer : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Scenario)
    QML_SINGLETON
    Q_PROPERTY(bool running READ running CONSTANT)

public:
    // Time allowed for the first frame before the run fails.
    static constexpr int kFirstFrameTimeoutMs = 15000;
    // How long a capture may take to write before its step fails.
    static constexpr int kCaptureTimeoutMs = 10000;

    ScenarioPlayer(pipeline::Pipeline *pipeline, ProcessingControl *processing,
        DetectionControl *detection, CaptureControl *captures, QObject *parent = nullptr);

    static ScenarioPlayer *create(QQmlEngine *, QJSEngine *engine);
    inline static ScenarioPlayer *instance_ = nullptr;

    // Plays `scenario` once the first frame arrives; results go to
    // `resultsPath` as JSON (none if empty).
    void play(Scenario scenario, QString name, QString resultsPath);
    bool running() const { return !scenario_.steps.isEmpty(); }

signals:
    void modeRequested(const QString &mode);

private:
    void runStep();
    void finishStep(bool passed, const QString &detail, const QString &capture = QString());
    void waitFor(int timeoutMs, const QString &timeoutDetail);
    void finish();

    QPointer<pipeline::Pipeline> pipeline_;
    QPointer<ProcessingControl> processing_;
    QPointer<DetectionControl> detection_;
    QPointer<CaptureControl> captures_;

    Scenario scenario_;
    QString name_;
    QString resultsPath_;
    qsizetype current_ = 0;
    int failures_ = 0;
    int framesSeen_ = 0;
    QJsonArray results_;
    QElapsedTimer clock_;     // since the first frame
    QElapsedTimer stepClock_; // since the current step started
    QTimer *timer_ = nullptr; // a step's wait or time limit
    // The current step's own connections (frames, detections, capture results).
    QList<QMetaObject::Connection> stepConnections_;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_SCENARIOPLAYER_H
