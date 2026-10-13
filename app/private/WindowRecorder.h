#ifndef PIVISION_APP_WINDOWRECORDER_H
#define PIVISION_APP_WINDOWRECORDER_H

#include <QElapsedTimer>
#include <QFile>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QTextStream>

class QQuickWindow;
class QTimer;

namespace pivision::app {

// Saves what the window shows as numbered JPEG frames (frame-00001.jpg, ...),
// for turning into a GIF afterwards (scripts/pv gif). Grabbing reads the
// picture back from the GPU, which costs time on a Pi, so it's for demo runs,
// never for measurements.
//
// Starts with the first video frame (before that the window is empty). A Pi
// can't always grab at kFps, so each frame's time since the start goes into
// times.txt (milliseconds, one line per frame) and the GIF plays at the real
// speed.
class WindowRecorder : public QObject {
public:
    // Frames per second; a GIF at this rate looks smooth enough and stays small.
    static constexpr int kFps = 8;
    static constexpr int kJpegQuality = 90;

    WindowRecorder(QQuickWindow *window, QString directory, QObject *parent = nullptr);

    // Begins recording once the window has next been drawn, so the first
    // grab already shows the frame that triggered it. Connect the first
    // frame's arrival here.
    void start();

private:
    void begin();
    void grab();

    QPointer<QQuickWindow> window_;
    QString directory_;
    QTimer *timer_ = nullptr;
    QElapsedTimer clock_;
    QFile times_;
    QTextStream timesOut_;
    int frame_ = 0;
    // The first grab comes back without the video in it (seen on the desktop
    // and the Pi), so it is thrown away.
    bool firstGrabDone_ = false;
};

} // namespace pivision::app

#endif // PIVISION_APP_WINDOWRECORDER_H
