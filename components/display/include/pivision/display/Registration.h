#ifndef PIVISION_DISPLAY_REGISTRATION_H
#define PIVISION_DISPLAY_REGISTRATION_H

#include <pivision/capture/SourceFactory.h>

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

} // namespace pivision::display

#endif // PIVISION_DISPLAY_REGISTRATION_H
