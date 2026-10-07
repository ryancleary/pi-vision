#ifndef PIVISION_DISPLAY_FRAMEVIEW_H
#define PIVISION_DISPLAY_FRAMEVIEW_H

#include <QImage>
#include <QMetaObject>
#include <QPointer>
#include <QQuickItem>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/Pipeline.h>

namespace pivision::display {

// Draws the newest frame from a Pipeline, scaled to fit inside the item.
// Frames are uploaded as scene-graph textures; no Qt Multimedia.
class FrameView : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(pivision::pipeline::Pipeline *pipeline READ pipeline WRITE setPipeline
            NOTIFY pipelineChanged)

public:
    explicit FrameView(QQuickItem *parent = nullptr);

    pivision::pipeline::Pipeline *pipeline() const;
    void setPipeline(pivision::pipeline::Pipeline *pipeline);

signals:
    void pipelineChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;

private:
    void onFrameAvailable();

    QPointer<pivision::pipeline::Pipeline> m_pipeline;
    QMetaObject::Connection m_connection;
    QImage m_frame;
    bool m_frameDirty = false;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_FRAMEVIEW_H

