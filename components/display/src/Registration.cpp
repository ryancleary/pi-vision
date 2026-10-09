#include <pivision/display/Registration.h>

#include "CaptureControl.h"
#include "MetricsMonitor.h"
#include "PipelineForeign.h"
#include "ProcessingControl.h"
#include "SourceSelector.h"
#include "UsbControl.h"

namespace pivision::display {

void exposePipeline(pivision::pipeline::Pipeline *pipeline)
{
    PipelineForeign::instance_ = pipeline;
}

void exposeSourceSelector(pivision::pipeline::Pipeline *pipeline,
    const pivision::capture::CameraConfig &camera,
    const pivision::capture::TestPatternConfig &pattern)
{
    SourceSelector::instance_ = new SourceSelector(pipeline, camera, pattern, pipeline);
}

void exposeProcessingControl(pivision::pipeline::Pipeline *pipeline,
    const QList<pivision::pipeline::StageConfig> &stages)
{
    ProcessingControl::instance_ = new ProcessingControl(pipeline, stages, pipeline);
}

void exposeMetricsMonitor(pivision::pipeline::Pipeline *pipeline,
    const QList<pivision::pipeline::StageConfig> &stages)
{
    MetricsMonitor::instance_ = new MetricsMonitor(pipeline, stages, pipeline);
}

void exposeCaptures(pivision::pipeline::Pipeline *pipeline, const QString &directory, int keep,
    const QString &appVersion)
{
    Q_ASSERT(MetricsMonitor::instance_ && ProcessingControl::instance_);
    CaptureControl::instance_ = new CaptureControl(pipeline, MetricsMonitor::instance_,
        ProcessingControl::instance_, pivision::snapshot::SnapshotStore(directory, keep), appVersion,
        pipeline);
}

void exposeUsb(const QString &mountRoot, const QString &captureDirectory,
    const QString &crashDirectory, QObject *parent)
{
    UsbControl::instance_ = new UsbControl(mountRoot, captureDirectory, crashDirectory, parent);
}

} // namespace pivision::display
