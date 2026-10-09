#include <pivision/processing/StageFactory.h>

#include "BuiltinStages.h"

namespace pivision::processing {

std::unique_ptr<Stage> makeStage(const std::string &id)
{
    if (id == "clahe")
        return std::make_unique<ClaheStage>();
    if (id == "grayscale")
        return std::make_unique<GrayscaleStage>();
    if (id == "blur")
        return std::make_unique<BlurStage>();
    if (id == "edges")
        return std::make_unique<EdgesStage>();
    return nullptr;
}

std::vector<std::string> knownStageIds()
{
    return { "clahe", "grayscale", "blur", "edges" };
}

} // namespace pivision::processing
