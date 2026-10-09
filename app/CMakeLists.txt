qt_add_executable(pivision
    src/main.cpp
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
    FILES config/stages.json
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
)

install(TARGETS pivision
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

