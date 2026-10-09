#include <pivision/display/Registration.h>

#include "PipelineForeign.h"
#include "SourceSelector.h"

namespace pivision::display {

void exposePipeline(pivision::pipeline::Pipeline *pipeline)
{
    PipelineForeign::s_instance = pipeline;
}

void exposeSourceSelector(pivision::pipeline::Pipeline *pipeline,
    const pivision::capture::CameraConfig &camera,
    const pivision::capture::TestPatternConfig &pattern)
{
    SourceSelector::s_instance = new SourceSelector(pipeline, camera, pattern, pipeline);
}

} // namespace pivision::display
