qt_add_library(pivision_display STATIC)
add_library(pivision::display ALIAS pivision_display)

qt_add_qml_module(pivision_display
    URI PiVision.Display
    VERSION 1.0
    SOURCES
        include/pivision/display/Registration.h
        private/FrameView.h
        private/PipelineForeign.h
        private/ProcessingControl.h
        private/SourceSelector.h
        src/FrameView.cpp
        src/ProcessingControl.cpp
        src/Registration.cpp
        src/SourceSelector.cpp
)

target_include_directories(pivision_display
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_display
    PUBLIC pivision::capture pivision::pipeline Qt6::Quick
    PRIVATE pivision::logging
)

