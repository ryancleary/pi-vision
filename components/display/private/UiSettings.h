#ifndef PIVISION_DISPLAY_UISETTINGS_H
#define PIVISION_DISPLAY_UISETTINGS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include <pivision/display/UiConfig.h>

class QQmlEngine;
class QJSEngine;

namespace pivision::display {

// The app layout settings (ui.json), for QML. Exposed as the singleton
// UiConfig through exposeUiConfig().
class UiSettings : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(UiConfig)
    QML_SINGLETON
    Q_PROPERTY(QStringList modeButtons READ modeButtons CONSTANT)
    Q_PROPERTY(QString defaultMode READ defaultMode CONSTANT)

public:
    explicit UiSettings(UiConfig config, QObject *parent = nullptr);

    static UiSettings *create(QQmlEngine *, QJSEngine *engine);
    inline static UiSettings *instance_ = nullptr;

    QStringList modeButtons() const { return config_.modeButtons; }
    QString defaultMode() const { return config_.defaultMode; }

private:
    UiConfig config_;
};

} // namespace pivision::display

#endif // PIVISION_DISPLAY_UISETTINGS_H
