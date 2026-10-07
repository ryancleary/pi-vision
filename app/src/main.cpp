#include <cstdlib>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#include <pivision/capture/SourceFactory.h>
#include <pivision/display/Registration.h>
#include <pivision/pipeline/Pipeline.h>

// PiVision.Display is a static QML module, so its plugin must be imported explicitly.
Q_IMPORT_QML_PLUGIN(PiVision_DisplayPlugin)

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    pivision::pipeline::Pipeline pipeline(pivision::capture::makeTestPatternSource());
    pivision::display::exposePipeline(&pipeline);

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(EXIT_FAILURE); }, Qt::QueuedConnection);
    engine.loadFromModule("PiVision", "Main");

    pipeline.start();
    return app.exec();
}
