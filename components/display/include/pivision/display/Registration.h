#ifndef PIVISION_DISPLAY_REGISTRATION_H
#define PIVISION_DISPLAY_REGISTRATION_H

#include <QList>
#include <QString>

#include <pivision/capture/SourceFactory.h>
#include <pivision/pipeline/StageConfig.h>

namespace pivision::pipeline {
class Pipeline;
}

namespace pivision::display {

// Makes `pipeline` available to QML as the singleton `Pipeline`.
// Call before loading any QML. The caller keeps ownership.
void exposePipeline(pivision::pipeline::Pipeline *pipeline);

// Makes the source dropdown's backend available to QML as the singleton
// `SourceSelector`. Sources picked in the UI are opened with `camera` and
// `pattern`. Call before loading any QML; the selector is owned by `pipeline`.
void exposeSourceSelector(pivision::pipeline::Pipeline *pipeline,
    const pivision::capture::CameraConfig &camera,
    const pivision::capture::TestPatternConfig &pattern);

// Makes the processing panel's backend available to QML as the singleton
// `ProcessingControl`, listing `stages` (the parsed stages.json) and applying
// changes to `pipeline`. Call before loading any QML; owned by `pipeline`.
void exposeProcessingControl(pivision::pipeline::Pipeline *pipeline,
    const QList<pivision::pipeline::StageConfig> &stages);

// Makes the Capture button's backend available to QML as the singleton
// `Captures`. Captures go into numbered folders under `directory`; only the
// newest `keep` are kept (0 keeps all). `appVersion` is recorded in each.
// Call after exposeProcessingControl() and exposeMetricsMonitor(), whose
// current values it records; owned by `pipeline`.
void exposeCaptures(pivision::pipeline::Pipeline *pipeline, const QString &directory, int keep,
    const QString &appVersion);

// Makes the metrics panel's backend available to QML as the singleton
// `MetricsMonitor`, reading `pipeline` and naming stages from `stages`.
// Call before loading any QML; owned by `pipeline`.
void exposeMetricsMonitor(pivision::pipeline::Pipeline *pipeline,
    const QList<pivision::pipeline::StageConfig> &stages);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_REGISTRATION_H
