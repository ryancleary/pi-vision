#include "UiSettings.h"

#include <utility>

#include <QJSEngine>

namespace pivision::display {

UiSettings::UiSettings(UiConfig config, QObject *parent)
    : QObject(parent)
    , config_(std::move(config))
{
}

UiSettings *UiSettings::create(QQmlEngine *, QJSEngine *engine)
{
    Q_ASSERT(instance_);
    Q_ASSERT(engine->thread() == instance_->thread());
    // C++ owns the object; stop the QML engine from deleting it.
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
    return instance_;
}

} // namespace pivision::display
