#ifndef PIVISION_DISPLAY_PIPELINEFOREIGN_H
#define PIVISION_DISPLAY_PIPELINEFOREIGN_H

#include <QJSEngine>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>

#include <pivision/pipeline/Pipeline.h>

namespace pivision::display {

// Registers the existing Pipeline object with QML as a singleton named Pipeline.
// The object is created in main.cpp and handed over through exposePipeline().
struct PipelineForeign {
    Q_GADGET
    QML_FOREIGN(pivision::pipeline::Pipeline)
    QML_SINGLETON
    QML_NAMED_ELEMENT(Pipeline)

public:
    static pivision::pipeline::Pipeline *create(QQmlEngine *, QJSEngine *engine)
    {
        Q_ASSERT(s_instance);
        Q_ASSERT(engine->thread() == s_instance->thread());
        // C++ owns the object; stop the QML engine from deleting it.
        QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
        return s_instance;
    }

    inline static pivision::pipeline::Pipeline *s_instance = nullptr;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_PIPELINEFOREIGN_H

