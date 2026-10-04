#include <cstdlib>
#include <memory>
#include <optional>

#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSize>
#include <QtQml/QQmlExtensionPlugin>

#include <opencv2/core/version.hpp>

#include <pivision/capture/SourceFactory.h>
#include <pivision/display/Registration.h>
#include <pivision/logging/Logging.h>
#include <pivision/pipeline/Pipeline.h>
#include <pivision/pipeline/StageConfig.h>

#include "TerminationHandler.h"
#include "Utils.h"
#include "WindowRecorder.h"

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
    const QString defaultModel
        = QDir(storage.modelDirectory).filePath(QString::fromLatin1(pivision::app::kDefaultModelDescription));
    const QCommandLineOption modelOption(QStringLiteral("model"),
        QStringLiteral("Detection model description <file>; the model file sits next to it "
                       "(default %1).")
            .arg(defaultModel),
        QStringLiteral("file"), defaultModel);
    const QCommandLineOption uiConfigOption(QStringLiteral("ui-config"),
        QStringLiteral("Layout overrides <file>, applied over the built-in ui.json%1.")
            .arg(storage.uiOverrides.isEmpty()
                    ? QString()
                    : QStringLiteral(" (default %1, if it exists)").arg(storage.uiOverrides)),
        QStringLiteral("file"), storage.uiOverrides);
    const QCommandLineOption cameraFormatOption(QStringLiteral("camera-format"),
        QStringLiteral("Camera pixel format to request: mjpeg (less USB bandwidth) or yuyv "
                       "(no JPEG decoding; default mjpeg)."),
        QStringLiteral("format"), QStringLiteral("mjpeg"));
    const QCommandLineOption scenarioOption(QStringLiteral("scenario"),
        QStringLiteral("Play the scenario <file> (a system test; see tests/system), then quit "
                       "with 0 if every check passed, 1 if not."),
        QStringLiteral("file"));
    const QCommandLineOption resultsOption(QStringLiteral("results"),
        QStringLiteral("With --scenario: write each step's result to <file> as JSON."),
        QStringLiteral("file"));
    const QCommandLineOption recordOption(QStringLiteral("record"),
        QStringLiteral("Save the window as JPEG frames in <dir>, for a GIF (scripts/pv gif)."),
        QStringLiteral("dir"));
    const QCommandLineOption verboseOption(QStringLiteral("verbose"),
        QStringLiteral("Include debug messages in the log."));
    parser.addOptions({ cameraOption, patternOption, sizeOption, processSizeOption, logFileOption,
        captureDirOption, captureKeepOption, usbRootOption, modelOption, uiConfigOption,
        cameraFormatOption, scenarioOption, resultsOption, recordOption, verboseOption });
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

    // Layout settings (toolbar mode buttons, starting mode), likewise built in.
    QFile uiFile(QStringLiteral(":/pivision/config/ui.json"));
    QString uiError;
    auto ui = uiFile.open(QIODevice::ReadOnly)
        ? pivision::display::parseUiConfig(uiFile.readAll(), &uiError)
        : std::nullopt;
    if (!ui) {
        qCCritical(pivision::logging::lcApp) << "Invalid ui.json:" << uiError;
        return EXIT_FAILURE;
    }

    // A screen's own layout needs (a display profile on the device, or
    // --ui-config). The default file is optional; one named explicitly isn't.
    const QString uiOverridePath = parser.value(uiConfigOption);
    QFile uiOverrideFile(uiOverridePath);
    if (!uiOverridePath.isEmpty() && (uiOverrideFile.exists() || parser.isSet(uiConfigOption))) {
        ui = uiOverrideFile.open(QIODevice::ReadOnly)
            ? pivision::display::applyUiOverrides(*ui, uiOverrideFile.readAll(), &uiError)
            : std::nullopt;
        if (!ui) {
            qCCritical(pivision::logging::lcApp) << "Invalid" << uiOverridePath << ":"
                                                 << (uiError.isEmpty() ? uiOverrideFile.errorString() : uiError);
            return EXIT_FAILURE;
        }
        qCInfo(pivision::logging::lcApp) << "Layout overrides from" << uiOverridePath;
    }

    const QString cameraFormat = parser.value(cameraFormatOption);
    if (cameraFormat != QStringLiteral("mjpeg") && cameraFormat != QStringLiteral("yuyv"))
        return pivision::app::usageError(QStringLiteral("--camera-format must be mjpeg or yuyv"));

    // Read up front, so a broken file stops the run before anything starts.
    std::optional<pivision::display::Scenario> scenario;
    if (parser.isSet(scenarioOption)) {
        QFile scenarioFile(parser.value(scenarioOption));
        QString scenarioError;
        scenario = scenarioFile.open(QIODevice::ReadOnly)
            ? pivision::display::parseScenario(scenarioFile.readAll(), &scenarioError)
            : std::nullopt;
        if (!scenario) {
            qCCritical(pivision::logging::lcApp) << "Invalid scenario" << scenarioFile.fileName() << ":"
                                                 << (scenarioError.isEmpty() ? scenarioFile.errorString() : scenarioError);
            return EXIT_FAILURE;
        }
    }

    pivision::capture::CameraConfig camera;
    camera.width = size->width();
    camera.height = size->height();
    camera.mjpeg = cameraFormat == QStringLiteral("mjpeg");
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
    // Loaded the first time Detection mode is used.
    pipeline.setModelDescription(parser.value(modelOption));
    qCInfo(pivision::logging::lcApp, "Processing at up to %dx%d with %lld stage(s)",
        processSize->width(), processSize->height(), static_cast<long long>(stages->size()));
    pivision::display::exposePipeline(&pipeline);
    pivision::display::exposeUiConfig(*ui, &pipeline);
    pivision::display::exposeSourceSelector(&pipeline, camera, pattern);
    pivision::display::exposeProcessingControl(&pipeline, *stages);
    pivision::display::exposeMetricsMonitor(&pipeline, *stages);
    pivision::display::exposeDetection(&pipeline);
    pivision::display::exposeCaptures(&pipeline, parser.value(captureDirOption), captureKeep,
        QStringLiteral(PIVISION_VERSION));
    pivision::display::exposeUsb(parser.value(usbRootOption), parser.value(captureDirOption),
        storage.crashDirectory, &pipeline);
    pivision::display::exposeScenario(&pipeline, scenario,
        QFileInfo(parser.value(scenarioOption)).completeBaseName(), parser.value(resultsOption));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("PiVision", "Main");

    std::unique_ptr<pivision::app::WindowRecorder> recorder;
    if (parser.isSet(recordOption)) {
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().value(0));
        if (window)
            recorder = std::make_unique<pivision::app::WindowRecorder>(window, parser.value(recordOption));
    }

    // systemctl stop sends SIGTERM; quit normally so it isn't counted as a crash.
    pivision::app::TerminationHandler termination;
    pipeline.start();
    return app.exec();
}
