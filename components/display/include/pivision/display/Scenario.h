#ifndef PIVISION_DISPLAY_SCENARIO_H
#define PIVISION_DISPLAY_SCENARIO_H

#include <optional>

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

namespace pivision::display {

// A scripted run of the app (a system test): steps that do what a person
// would do in the UI, in order, plus checks. The input comes from the command
// line as usual (for the tests, OpenCV's vtest.avi clip), so the same steps run
// against the same frames every time.
//
//   { "about": "what this scenario is for",
//     "steps": [
//       { "do": "mode",           "value": "sideBySide" },
//       { "do": "stages",         "enabled": ["blur", "edges"] },
//       { "do": "detectionInput", "value": "processed" },
//       { "do": "wait",           "seconds": 10 },
//       { "do": "capture",        "label": "02 blur + edges" },
//       { "do": "expectFrames",   "atLeast": 30, "withinSeconds": 5 },
//       { "do": "expectDetection", "label": "person", "withinSeconds": 30 }
//     ] }
//
//   mode            switch the view, as the toolbar buttons do. "detection"
//                   also turns the detector on; any other mode turns it off.
//   stages          exactly these stages on, all others off (ids from stages.json)
//   detectionInput  "raw" or "processed"
//   wait            let things settle; metrics average over the last second
//   capture         a capture, as the button takes it, labelled in info.json
//   expectFrames    passes once this many frames reach the screen in time
//   expectDetection passes once the detector finds this class in time
//                   (needs "mode": "detection" first)
//
// Any step may have an "about" note; it is ignored. A failed check is
// recorded and the run carries on, so one run reports every failure.
struct ScenarioStep {
    enum class Action { Mode, Stages, DetectionInput, Wait, Capture, ExpectFrames, ExpectDetection };

    Action action = Action::Wait;
    QString value;          // Mode: view mode; DetectionInput: "raw" or "processed"
    QStringList stages;     // Stages: the ones to turn on
    QString label;          // Capture: label; ExpectDetection: class name
    double seconds = 0.0;   // Wait: how long; Expect*: time limit
    int atLeast = 0;        // ExpectFrames: frame count
    QString text;           // the step as written, for results and logs
};

struct Scenario {
    QString about;
    QList<ScenarioStep> steps;
};

// The scenario file's "do" names, in the order the comment above lists them.
QStringList knownScenarioActions();

// Reads a scenario file. Anything malformed is an error (unknown action,
// missing or wrong-typed field, unknown mode): a test that silently skips a
// step would pass for the wrong reason. Returns nothing and sets `error` if so.
std::optional<Scenario> parseScenario(const QByteArray &json, QString *error = nullptr);

} // namespace pivision::display

#endif // PIVISION_DISPLAY_SCENARIO_H
