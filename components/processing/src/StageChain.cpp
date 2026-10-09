#include <pivision/processing/StageChain.h>

#include <algorithm>
#include <chrono>
#include <utility>

namespace pivision::processing {

void StageChain::add(std::unique_ptr<Stage> stage, bool enabled)
{
    if (stage)
        entries_.push_back({ std::move(stage), enabled });
}

bool StageChain::setEnabled(const std::string &id, bool enabled)
{
    Entry *entry = find(id);
    if (!entry)
        return false;
    entry->enabled = enabled;
    return true;
}

bool StageChain::setParameter(const std::string &id, const std::string &name, double value)
{
    Entry *entry = find(id);
    return entry && entry->stage->setParameter(name, value);
}

bool StageChain::isEnabled(const std::string &id) const
{
    const Entry *entry = find(id);
    return entry && entry->enabled;
}

bool StageChain::anyEnabled() const
{
    return std::any_of(
        entries_.begin(), entries_.end(), [](const Entry &entry) { return entry.enabled; });
}

std::vector<std::string> StageChain::order() const
{
    std::vector<std::string> ids;
    ids.reserve(entries_.size());
    for (const Entry &entry : entries_)
        ids.push_back(entry.stage->id());
    return ids;
}

void StageChain::run(const cv::Mat &in, cv::Mat &out, std::vector<StageTiming> *timings)
{
    if (timings)
        timings->clear();

    // Stages alternate between two reusable buffers, so steady-state frames
    // allocate nothing.
    const cv::Mat *current = &in;
    int next = 0;
    for (Entry &entry : entries_) {
        if (!entry.enabled)
            continue;
        const auto start = std::chrono::steady_clock::now();
        entry.stage->process(*current, buffers_[next]);
        if (timings) {
            const std::chrono::duration<double, std::milli> elapsed
                = std::chrono::steady_clock::now() - start;
            timings->push_back({ entry.stage->id(), elapsed.count() });
        }
        current = &buffers_[next];
        next = 1 - next;
    }
    current->copyTo(out);
}

StageChain::Entry *StageChain::find(const std::string &id)
{
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&](const Entry &entry) { return entry.stage->id() == id; });
    return it == entries_.end() ? nullptr : &*it;
}

const StageChain::Entry *StageChain::find(const std::string &id) const
{
    return const_cast<StageChain *>(this)->find(id);
}

} // namespace pivision::processing
