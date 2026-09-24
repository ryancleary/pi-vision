qt_add_executable(pivision
    src/main.cpp
    src/Utils.cpp
    src/Utils.h
)

# Shared colors and sizes, used as "Theme" from any QML file in the module.
set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(pivision
    URI PiVision
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/PanelButton.qml
        qml/CompareView.qml
        qml/MetricsPanel.qml
        qml/StageControl.qml
)

# Processing stages, in order, with defaults and slider ranges. Compiled in:
# changing it means rebuilding.
qt_add_resources(pivision "config"
    PREFIX /pivision
    FILES config/stages.json config/ui.json
)

target_link_libraries(pivision PRIVATE
    Qt6::Quick
    pivision::capture
    pivision::pipeline
    pivision::display
    pivision::logging
    pivision_displayplugin
)

target_compile_definitions(pivision PRIVATE
    PIVISION_VERSION="${PROJECT_VERSION}"
    PIVISION_DEV_LOG_FILE="${PIVISION_DEV_LOG_FILE}"
    PIVISION_DEV_CAPTURE_DIR="${PIVISION_DEV_CAPTURE_DIR}"
    PIVISION_DEV_MODEL_DIR="${PIVISION_DEV_MODEL_DIR}"
)

install(TARGETS pivision
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# The detection model's description, where the app looks on the device
# (/usr/share/pivision/models). The model file itself is installed by the
# pivision-model-nanodet recipe, since it isn't in git.
install(FILES ${PROJECT_SOURCE_DIR}/assets/models/nanodet-plus-m-1.5x-416.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pivision/models
)

