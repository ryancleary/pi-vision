#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QRegularExpression>
#include <QSize>
#include <QtQml/QQmlExtensionPlugin>

#include <pivision/capture/SourceFactory.h>
#include <pivision/display/Registration.h>
#include <pivision/pipeline/Pipeline.h>

// PiVision.Display is a static QML module, so its plugin must be imported explicitly.
Q_IMPORT_QML_PLUGIN(PiVision_DisplayPlugin)

namespace {

int usageError(const QString &message)
{
    std::fprintf(stderr, "pivision: %s\nTry --help.\n", qPrintable(message));
    return EXIT_FAILURE;
}

std::optional<QSize> parseSize(const QString &text)
{
    static const QRegularExpression pattern(QStringLiteral("^(\\d+)x(\\d+)$"));
    const auto match = pattern.match(text);
    if (!match.hasMatch())
        return std::nullopt;
    const QSize size(match.captured(1).toInt(), match.captured(2).toInt());
    return size.isEmpty() ? std::nullopt : std::optional<QSize>(size);
}

} // namespace

int main(int argc, char *argv[])
{
    using namespace pivision::capture;

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
    parser.addOptions({ cameraOption, patternOption, sizeOption });
    parser.addPositionalArgument(QStringLiteral("source"),
        QStringLiteral("Video file, stream URL or /dev/video* device."), QStringLiteral("[source]"));
    parser.process(app);

    const auto size = parseSize(parser.value(sizeOption));
    if (!size)
        return usageError(QStringLiteral("--size must look like 640x480"));

    CameraConfig camera;
    camera.width = size->width();
    camera.height = size->height();
    TestPatternConfig pattern;
    pattern.width = size->width();
    pattern.height = size->height();

    const QStringList positional = parser.positionalArguments();
    const int chosen = int(parser.isSet(cameraOption)) + int(parser.isSet(patternOption))
        + int(!positional.isEmpty());
    if (chosen > 1 || positional.size() > 1)
        return usageError(QStringLiteral("give at most one source"));

    std::unique_ptr<FrameSource> source;
    if (parser.isSet(patternOption)) {
        source = makeTestPatternSource(pattern);
    } else if (parser.isSet(cameraOption)) {
        QString device = parser.value(cameraOption);
        bool isNumber = false;
        const int number = device.toInt(&isNumber);
        if (isNumber)
            device = QStringLiteral("/dev/video%1").arg(number);
        source = makeCameraSource(device.toStdString(), camera);
    } else if (!positional.isEmpty()) {
        source = makeSourceFromSpec(positional.first().toStdString(), camera);
    } else {
        source = makeAutoSource(camera, pattern);
    }

    pivision::pipeline::Pipeline pipeline(std::move(source));
    pivision::display::exposePipeline(&pipeline);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("PiVision", "Main");

    pipeline.start();
    return app.exec();
}
