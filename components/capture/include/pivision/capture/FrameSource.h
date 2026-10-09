#ifndef PIVISION_CAPTURE_FRAMESOURCE_H
#define PIVISION_CAPTURE_FRAMESOURCE_H

#include <string>

#include <pivision/capture/Frame.h>

namespace pivision::capture {

// Interface for anything that produces frames.
// Not thread-safe: one thread owns a source and calls it.
class FrameSource {
public:
    virtual ~FrameSource();

    FrameSource(const FrameSource &) = delete;
    FrameSource &operator=(const FrameSource &) = delete;

    // Prepares the source. Returns false if it can't be used.
    virtual bool open() = 0;

    // Fills `out` with the next frame, reusing its buffer when possible.
    // Returns false if no frame is available.
    virtual bool read(Frame &out) = 0;

    virtual void close() = 0;

    // Name shown in the UI.
    virtual std::string name() const = 0;

    // Rate the source should be read at.
    virtual double nominalFps() const = 0;

protected:
    FrameSource() = default;
};

} // namespace pivision::capture

#endif // PIVISION_CAPTURE_FRAMESOURCE_H
