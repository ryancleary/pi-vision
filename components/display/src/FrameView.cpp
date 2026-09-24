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
            connect(pipeline_.data(), &pipeline::Pipeline::detectionAvailable, this,
                &FrameView::onDetectionAvailable),
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
    if (!pipeline_ || stream_ == Detected)
        return;
    if (auto frame = pipeline_->latestFrame())
        showImage(stream_ == Raw ? frame->raw : frame->processed);
}

void FrameView::onDetectionAvailable()
{
    if (!pipeline_ || stream_ != Detected)
        return;
    if (auto result = pipeline_->latestDetection())
        showImage(result->image);
    else
        clearFrame(); // detection switched off or reset
}

void FrameView::showImage(const QImage &image)
{
    // Shared, not copied: QImage is implicitly shared.
    frame_ = image;
    frameDirty_ = true;
    updateImageRect();
    update(); // schedules updatePaintNode() on the render thread
}

void FrameView::updateImageRect()
{
    // Show the whole frame as large as the item allows, centered, with bars
    // on the sides that don't fill ("letterboxing"). QSizeF::scaled with
    // Qt::KeepAspectRatio computes the size; moveCenter does the centering.
    QRectF fitted;
    if (!frame_.isNull()) {
        fitted = QRectF(QPointF(), QSizeF(frame_.size()).scaled(size(), Qt::KeepAspectRatio));
        fitted.moveCenter(boundingRect().center());
    }
    if (fitted == imageRect_)
        return;
    imageRect_ = fitted;
    emit imageRectChanged();
}

void FrameView::setStream(Stream stream)
{
    if (stream == stream_)
        return;
    stream_ = stream;
    // Show the other image right away rather than at the next frame.
    if (stream_ == Detected)
        onDetectionAvailable();
    else
        onFrameAvailable();
    emit streamChanged();
}

void FrameView::clearFrame()
{
    frame_ = QImage();
    frameDirty_ = true;
    updateImageRect();
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

    node->setRect(imageRect_); // see updateImageRect()

    return node;
}

} // namespace pivision::display
