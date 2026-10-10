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
// raw camera image, the processed one, or the image of the newest detection. Several views can show the same
// pipeline. Frames are uploaded as scene-graph textures; no Qt Multimedia.
class FrameView : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(pivision::pipeline::Pipeline *pipeline READ pipeline WRITE setPipeline NOTIFY
            pipelineChanged)
    Q_PROPERTY(Stream stream READ stream WRITE setStream NOTIFY streamChanged)
    // Where the image is drawn inside the item, in the item's coordinates
    // (smaller than the item where the image doesn't fill it). Empty with no image.
    Q_PROPERTY(QRectF imageRect READ imageRect NOTIFY imageRectChanged)

public:
    enum Stream {
        Processed,
        Raw,
        Detected, // the image the newest detection ran on
    };
    Q_ENUM(Stream)

    explicit FrameView(QQuickItem *parent = nullptr);

    pivision::pipeline::Pipeline *pipeline() const { return pipeline_; }
    void setPipeline(pivision::pipeline::Pipeline *pipeline);

    Stream stream() const { return stream_; }
    void setStream(Stream stream);

    QRectF imageRect() const { return imageRect_; }

signals:
    void pipelineChanged();
    void streamChanged();
    void imageRectChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

    // Refit the image when the item is resized, not only when the next frame
    // arrives, so it follows the compare view's animations smoothly.
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override
    {
        QQuickItem::geometryChange(newGeometry, oldGeometry);
        updateImageRect();
        update();
    }

private:
    void onFrameAvailable();
    void onDetectionAvailable();
    void showImage(const QImage &image);
    void clearFrame();
    void updateImageRect();

    QPointer<pivision::pipeline::Pipeline> pipeline_;
    // Connections to the current pipeline, dropped when it changes.
    QList<QMetaObject::Connection> connections_;
    Stream stream_ = Processed;
    QImage frame_;
    QRectF imageRect_;
    bool frameDirty_ = false;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_FRAMEVIEW_H
