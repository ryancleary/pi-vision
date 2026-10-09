#include "FrameView.h"

#include <utility>

#include <QQuickWindow>
#include <QSGSimpleTextureNode>

namespace pivision::display {

FrameView::FrameView(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

void FrameView::setPipeline(pipeline::Pipeline *pipeline)
{
    if (pipeline_ == pipeline)
        return;

    for (const auto &connection : std::as_const(connections_))
        disconnect(connection);
    connections_.clear();

    pipeline_ = pipeline;
    if (pipeline_) {
        connections_ = {
            connect(pipeline_.data(), &pipeline::Pipeline::frameAvailable, this, &FrameView::onFrameAvailable),
            // A source switch or failure cuts the feed: show nothing rather than a frozen frame.
            connect(pipeline_.data(), &pipeline::Pipeline::cleared, this, &FrameView::clearFrame),
            connect(pipeline_.data(), &pipeline::Pipeline::failed, this, &FrameView::clearFrame),
        };
    }
    clearFrame();
    emit pipelineChanged();
}

void FrameView::onFrameAvailable()
{
    if (!pipeline_)
        return;

    if (auto frame = pipeline_->latestFrame()) {
        // Shared, not copied: QImage is implicitly shared.
        frame_ = stream_ == Raw ? frame->raw : frame->processed;
        frameDirty_ = true;
        update(); // schedules updatePaintNode() on the render thread
    }
}

void FrameView::setStream(Stream stream)
{
    if (stream == stream_)
        return;
    stream_ = stream;
    onFrameAvailable(); // show the other image of the current frame right away
    emit streamChanged();
}

void FrameView::clearFrame()
{
    frame_ = QImage();
    frameDirty_ = true;
    update();
}

QSGNode *FrameView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Runs on the render thread while the GUI thread is blocked,
    // so reading frame_ here is safe.
    auto *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (frame_.isNull() || width() <= 0 || height() <= 0) {
        delete node;
        return nullptr;
    }

    if (!node) {
        node = new QSGSimpleTextureNode;
        node->setOwnsTexture(true); // the old texture is deleted when replaced
        node->setFiltering(QSGTexture::Linear);
        frameDirty_ = true;
    }

    if (frameDirty_) {
        node->setTexture(window()->createTextureFromImage(frame_));
        frameDirty_ = false;
    }

    // Show the whole frame as large as the item allows, centered, with bars
    // on the sides that don't fill ("letterboxing"). QSizeF::scaled with
    // Qt::KeepAspectRatio computes the size; moveCenter does the centering.
    const QSizeF fitted = QSizeF(frame_.size()).scaled(size(), Qt::KeepAspectRatio);
    QRectF target(QPointF(), fitted);
    target.moveCenter(boundingRect().center());
    node->setRect(target);

    return node;
}

} // namespace pivision::display
