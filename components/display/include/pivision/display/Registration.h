#ifndef PIVISION_DISPLAY_REGISTRATION_H
#define PIVISION_DISPLAY_REGISTRATION_H

namespace pivision::pipeline {
class Pipeline;
}

namespace pivision::display {

// Makes `pipeline` available to QML as the singleton `Pipeline`.
// Call before loading any QML. The caller keeps ownership.
void exposePipeline(pivision::pipeline::Pipeline *pipeline);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_REGISTRATION_H

