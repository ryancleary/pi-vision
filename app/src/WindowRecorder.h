#ifndef PIVISION_APP_WINDOWRECORDER_H
#define PIVISION_APP_WINDOWRECORDER_H

#include <QObject>
#include <QPointer>
#include <QString>

class QQuickWindow;
class QTimer;

namespace pivision::app {

// Saves what the window shows as numbered JPEG frames (frame-00001.jpg, ...),
// for turning into a GIF afterwards (scripts/pv gif). Grabbing reads the
// picture back from the GPU, which costs time on a Pi, so it's for demo runs,
// never for measurements.
class WindowRecorder : public QObject {
public:
    // Frames per second; a GIF at this rate looks smooth enough and stays small.
    static constexpr int kFps = 8;
    static constexpr int kJpegQuality = 90;

    WindowRecorder(QQuickWindow *window, QString directory, QObject *parent = nullptr);

private:
    void grab();

    QPointer<QQuickWindow> window_;
    QString directory_;
    QTimer *timer_ = nullptr;
    int frame_ = 0;
};

} // namespace pivision::app

#endif // PIVISION_APP_WINDOWRECORDER_H
