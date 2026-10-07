#include <pivision/display/Registration.h>

#include "PipelineForeign.h"

namespace pivision::display {

void exposePipeline(pivision::pipeline::Pipeline *pipeline)
{
    PipelineForeign::s_instance = pipeline;
}

} // namespace pivision::display
