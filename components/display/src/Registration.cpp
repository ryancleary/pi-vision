#include <pivision/display/Registration.h>

#include "PipelineForeign.h"
#include "ProcessingControl.h"
#include "SourceSelector.h"

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

} // namespace pivision::display
