qt_add_executable(pivision
    src/main.cpp
)

qt_add_qml_module(pivision
    URI PiVision
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(pivision PRIVATE Qt6::Quick)

install(TARGETS pivision
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
