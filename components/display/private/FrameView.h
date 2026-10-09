#ifndef PIVISION_DISPLAY_FRAMEVIEW_H
#define PIVISION_DISPLAY_FRAMEVIEW_H

#include <QImage>
#include <QList>
#include <QMetaObject>
#include <QPointer>
#include <QQuickItem>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/Pipeline.h>

namespace pivision::display {

// Draws the newest frame from a Pipeline, scaled to fit inside the item: the
// raw camera image or the processed one. Several views can show the same
// pipeline. Frames are uploaded as scene-graph textures; no Qt Multimedia.
class FrameView : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(pivision::pipeline::Pipeline *pipeline READ pipeline WRITE setPipeline NOTIFY
            pipelineChanged)
    Q_PROPERTY(Stream stream READ stream WRITE setStream NOTIFY streamChanged)

public:
    enum Stream { Processed, Raw };
    Q_ENUM(Stream)

    explicit FrameView(QQuickItem *parent = nullptr);

    pivision::pipeline::Pipeline *pipeline() const { return pipeline_; }
    void setPipeline(pivision::pipeline::Pipeline *pipeline);

    Stream stream() const { return stream_; }
    void setStream(Stream stream);

signals:
    void pipelineChanged();
    void streamChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

    // Refit the image when the item is resized, not only when the next frame
    // arrives, so it follows the compare view's animations smoothly.
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override
    {
        QQuickItem::geometryChange(newGeometry, oldGeometry);
        update();
    }

private:
    void onFrameAvailable();
    void clearFrame();

    QPointer<pivision::pipeline::Pipeline> pipeline_;
    // Connections to the current pipeline, dropped when it changes.
    QList<QMetaObject::Connection> connections_;
    Stream stream_ = Processed;
    QImage frame_;
    bool frameDirty_ = false;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_FRAMEVIEW_H
