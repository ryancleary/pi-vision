#ifndef PIVISION_PIPELINE_DETECTIONWORKER_H
#define PIVISION_PIPELINE_DETECTIONWORKER_H

#include <memory>
#include <string>

#include <QObject>
#include <QString>

#include <pivision/detection/Detector.h>

#include "DetectionJob.h"

namespace pivision::pipeline {

// Lives on the detection thread. Takes the newest job, runs the detector on
// it, and puts the result in the output buffer. The model is loaded the first
// time a job arrives, here on this thread, so the UI never waits for it.
class DetectionWorker : public QObject {
    Q_OBJECT
public:
    DetectionWorker(DetectionJobBuffer &input, DetectionResultBuffer &output);
    ~DetectionWorker() override;

    // The model description file (see assets/models/). Takes effect at the
    // next load; call before detection is first enabled.
    void setModelDescription(const std::string &path) { modelDescription_ = path; }

public slots:
    void detectLatest();

signals:
    void resultAvailable();
    void stateChanged(pivision::pipeline::DetectorState state, const QString &error);

private:
    bool loadModel();

    DetectionJobBuffer &input_;
    DetectionResultBuffer &output_;
    std::string modelDescription_;
    std::unique_ptr<detection::Detector> detector_;
    bool loadFailed_ = false; // don't retry a broken model on every frame
};

} // namespace pivision::pipeline

#endif // PIVISION_PIPELINE_DETECTIONWORKER_H
