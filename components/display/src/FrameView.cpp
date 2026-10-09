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

pivision::pipeline::Pipeline *FrameView::pipeline() const { return m_pipeline; }

void FrameView::setPipeline(pivision::pipeline::Pipeline *pipeline)
{
    if (m_pipeline == pipeline)
        return;

    for (const auto &connection : std::as_const(m_connections))
        disconnect(connection);
    m_connections.clear();

    m_pipeline = pipeline;
    if (m_pipeline) {
        using pivision::pipeline::Pipeline;
        m_connections = {
            connect(m_pipeline.data(), &Pipeline::frameAvailable, this, &FrameView::onFrameAvailable),
            // A source switch or failure cuts the feed: show nothing rather than a frozen frame.
            connect(m_pipeline.data(), &Pipeline::cleared, this, &FrameView::clearFrame),
            connect(m_pipeline.data(), &Pipeline::failed, this, &FrameView::clearFrame),
        };
    }
    clearFrame();
    emit pipelineChanged();
}

void FrameView::onFrameAvailable()
{
    if (!m_pipeline)
        return;

    if (auto frame = m_pipeline->takeLatestFrame()) {
        m_frame = std::move(frame->image);
        m_frameDirty = true;
        update(); // schedules updatePaintNode() on the render thread
    }
}

void FrameView::clearFrame()
{
    m_frame = QImage();
    m_frameDirty = true;
    update();
}

QSGNode *FrameView::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    // Runs on the render thread while the GUI thread is blocked,
    // so reading m_frame here is safe.
    auto *node = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (m_frame.isNull() || width() <= 0 || height() <= 0) {
        delete node;
        return nullptr;
    }

    if (!node) {
        node = new QSGSimpleTextureNode;
        node->setOwnsTexture(true); // the old texture is deleted when replaced
        node->setFiltering(QSGTexture::Linear);
        m_frameDirty = true;
    }

    if (m_frameDirty) {
        node->setTexture(window()->createTextureFromImage(m_frame));
        m_frameDirty = false;
    }

    // Fit: scale to the largest size that shows the whole frame, centered.
    const QSizeF frameSize = m_frame.size();
    const qreal scale = qMin(width() / frameSize.width(), height() / frameSize.height());
    const QSizeF fitted = frameSize * scale;
    node->setRect(QRectF((width() - fitted.width()) / 2, (height() - fitted.height()) / 2,
        fitted.width(), fitted.height()));

    return node;
}

} // namespace pivision::display
