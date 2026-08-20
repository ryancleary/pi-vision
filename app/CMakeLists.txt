qt_add_executable(pivision
    src/main.cpp
)

qt_add_qml_module(pivision
    URI PiVision
    VERSION 1.0
    QML_FILES
        qml/Main.qml
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
)

install(TARGETS pivision
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

