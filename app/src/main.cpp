#include <cstdlib>
#include <memory>
#include <optional>

#include <QCommandLineParser>
#include <QFile>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSize>
#include <QtQml/QQmlExtensionPlugin>

#include <opencv2/core/version.hpp>

#include <pivision/capture/SourceFactory.h>
#include <pivision/display/Registration.h>
#include <pivision/logging/Logging.h>
#include <pivision/pipeline/Pipeline.h>
#include <pivision/pipeline/StageConfig.h>

#include "Utilities.h"

// PiVision.Display is a static QML module, so its plugin must be imported explicitly.
Q_IMPORT_QML_PLUGIN(PiVision_DisplayPlugin)

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("pivision"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(
        "Real-time vision pipeline. With no source given, uses the first camera that "
        "opens, or the test pattern if there is none."));
    parser.addHelpOption();
    const QCommandLineOption cameraOption(QStringLiteral("camera"),
        QStringLiteral("Use camera <device>: a number (0 for /dev/video0) or a path."),
        QStringLiteral("device"));
    const QCommandLineOption patternOption(QStringLiteral("test-pattern"),
        QStringLiteral("Use the built-in test pattern."));
    const QCommandLineOption sizeOption(QStringLiteral("size"),
        QStringLiteral("Capture size to request, WIDTHxHEIGHT (default 640x480)."),
        QStringLiteral("size"), QStringLiteral("640x480"));
    const QCommandLineOption processSizeOption(QStringLiteral("process-size"),
        QStringLiteral("Largest size to process at, WIDTHxHEIGHT (default 320x240, as on the Pi)."),
        QStringLiteral("size"), QStringLiteral("320x240"));
    const QCommandLineOption logFileOption(QStringLiteral("log-file"),
        QStringLiteral("Also write the log to <file>, replacing it each run.%1")
            .arg(QStringLiteral(PIVISION_DEV_LOG_FILE).isEmpty()
                    ? QString()
                    : QStringLiteral(" Default: %1").arg(QStringLiteral(PIVISION_DEV_LOG_FILE))),
        QStringLiteral("file"), QStringLiteral(PIVISION_DEV_LOG_FILE));
    const pivision::app::StorageDefaults storage = pivision::app::storageDefaults();
    const QCommandLineOption captureDirOption(QStringLiteral("capture-dir"),
        QStringLiteral("Save captures in <dir> (default %1).").arg(storage.captureDirectory),
        QStringLiteral("dir"), storage.captureDirectory);
    const QCommandLineOption captureKeepOption(QStringLiteral("capture-keep"),
        QStringLiteral("Keep only the newest <n> captures; 0 keeps all (default %1).")
            .arg(storage.captureKeep),
        QStringLiteral("n"), QString::number(storage.captureKeep));
    const QCommandLineOption usbRootOption(QStringLiteral("usb-root"),
        QStringLiteral("Look for USB sticks mounted directly below <dir> (default %1).")
            .arg(storage.usbRoot),
        QStringLiteral("dir"), storage.usbRoot);
    const QCommandLineOption verboseOption(QStringLiteral("verbose"),
        QStringLiteral("Include debug messages in the log."));
    parser.addOptions({ cameraOption, patternOption, sizeOption, processSizeOption, logFileOption,
        captureDirOption, captureKeepOption, usbRootOption, verboseOption });
    parser.addPositionalArgument(QStringLiteral("source"),
        QStringLiteral("Video file, stream URL or /dev/video* device."), QStringLiteral("[source]"));
    parser.process(app);

    pivision::logging::install(parser.value(logFileOption));
    if (parser.isSet(verboseOption))
        pivision::logging::enableVerbose();
    qCInfo(pivision::logging::lcApp, "pi-vision %s (Qt %s, OpenCV %s)",
        PIVISION_VERSION, qVersion(), CV_VERSION);
    if (!parser.value(logFileOption).isEmpty())
        qCInfo(pivision::logging::lcApp) << "Logging to" << parser.value(logFileOption);

    const auto size = pivision::app::parseSize(parser.value(sizeOption));
    if (!size)
        return pivision::app::usageError(QStringLiteral("--size must look like 640x480"));

    const auto processSize = pivision::app::parseSize(parser.value(processSizeOption));
    if (!processSize)
        return pivision::app::usageError(QStringLiteral("--process-size must look like 320x240"));

    bool keepIsNumber = false;
    const int captureKeep = parser.value(captureKeepOption).toInt(&keepIsNumber);
    if (!keepIsNumber || captureKeep < 0)
        return pivision::app::usageError(QStringLiteral("--capture-keep must be 0 or more"));

    // The processing stages ship inside the app; a bad file is a build mistake.
    QFile stageFile(QStringLiteral(":/pivision/config/stages.json"));
    QString stageError;
    const auto stages = stageFile.open(QIODevice::ReadOnly)
        ? pivision::pipeline::parseStageConfig(stageFile.readAll(), &stageError)
        : std::nullopt;
    if (!stages) {
        qCCritical(pivision::logging::lcApp) << "Invalid stages.json:" << stageError;
        return EXIT_FAILURE;
    }

    pivision::capture::CameraConfig camera;
    camera.width = size->width();
    camera.height = size->height();
    pivision::capture::TestPatternConfig pattern;
    pattern.width = size->width();
    pattern.height = size->height();

    const QStringList positional = parser.positionalArguments();
    // At most one of --camera, --test-pattern or a positional source may be given.
    const int chosen = static_cast<int>(parser.isSet(cameraOption))
        + static_cast<int>(parser.isSet(patternOption)) + static_cast<int>(!positional.isEmpty());
    if (chosen > 1 || positional.size() > 1)
        return pivision::app::usageError(QStringLiteral("give at most one source"));

    std::unique_ptr<pivision::capture::FrameSource> source;
    if (parser.isSet(patternOption)) {
        source = pivision::capture::makeTestPatternSource(pattern);
    } else if (parser.isSet(cameraOption)) {
        QString device = parser.value(cameraOption);
        bool isNumber = false;
        const int number = device.toInt(&isNumber);
        if (isNumber)
            device = QStringLiteral("/dev/video%1").arg(number);
        source = pivision::capture::makeCameraSource(device.toStdString(), camera);
    } else if (!positional.isEmpty()) {
        source = pivision::capture::makeSourceFromSpec(positional.first().toStdString(), camera);
    } else {
        source = pivision::capture::makeAutoSource(camera, pattern);
    }

    pivision::pipeline::Pipeline pipeline(std::move(source));
    pipeline.setStages(*stages);
    pipeline.setProcessSize(*processSize);
    qCInfo(pivision::logging::lcApp, "Processing at up to %dx%d with %lld stage(s)",
        processSize->width(), processSize->height(), static_cast<long long>(stages->size()));
    pivision::display::exposePipeline(&pipeline);
    pivision::display::exposeSourceSelector(&pipeline, camera, pattern);
    pivision::display::exposeProcessingControl(&pipeline, *stages);
    pivision::display::exposeMetricsMonitor(&pipeline, *stages);
    pivision::display::exposeCaptures(&pipeline, parser.value(captureDirOption), captureKeep,
        QStringLiteral(PIVISION_VERSION));
    pivision::display::exposeUsb(parser.value(usbRootOption), parser.value(captureDirOption),
        storage.crashDirectory, &pipeline);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("PiVision", "Main");

    pipeline.start();
    return app.exec();
}
