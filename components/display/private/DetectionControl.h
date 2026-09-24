#ifndef PIVISION_DISPLAY_DETECTIONCONTROL_H
#define PIVISION_DISPLAY_DETECTIONCONTROL_H

#include <QObject>
#include <QPointer>
#include <QString>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/Pipeline.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

// Backs Detection mode: switches detection on and off, picks its input, and
// gives QML the newest boxes. Exposed to QML as the singleton Detection
// through exposeDetection().
class DetectionControl : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Detection)
    QML_SINGLETON
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(Input input READ input WRITE setInput NOTIFY inputChanged)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    // The newest result's boxes: [ { x, y, width, height, label, score } ].
    // Position and size are fractions (0..1) of the image the detection ran
    // on, so they can be placed on any view of the same picture.
    Q_PROPERTY(QVariantList boxes READ boxes NOTIFY resultChanged)
    // Width / height of the model's input (1 for a square model), for drawing
    // the model's view in the detected-frame view; 0 before the first result.
    Q_PROPERTY(double modelAspect READ modelAspect NOTIFY resultChanged)

public:
    enum Input { Raw, Processed };
    Q_ENUM(Input)
    enum State { Off, Loading, Ready, Failed };
    Q_ENUM(State)

    explicit DetectionControl(pipeline::Pipeline *pipeline, QObject *parent = nullptr);

    static DetectionControl *create(QQmlEngine *, QJSEngine *engine);
    inline static DetectionControl *instance_ = nullptr;

    bool enabled() const { return pipeline_ && pipeline_->detectionEnabled(); }
    void setEnabled(bool enabled);
    Input input() const;
    void setInput(Input input);
    State state() const;
    QString error() const { return pipeline_ ? pipeline_->detectorError() : QString(); }
    QVariantList boxes() const { return boxes_; }
    double modelAspect() const { return modelAspect_; }

signals:
    void enabledChanged();
    void inputChanged();
    void stateChanged();
    void resultChanged();

private:
    void onDetectionAvailable();

    QPointer<pipeline::Pipeline> pipeline_;
    QVariantList boxes_;
    double modelAspect_ = 0.0;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_DETECTIONCONTROL_H
