add_library(pivision_capture STATIC
    include/pivision/capture/Frame.h
    include/pivision/capture/FrameSource.h
    include/pivision/capture/SourceFactory.h
    private/TestPatternSource.h
    src/FrameSource.cpp
    src/SourceFactory.cpp
    src/TestPatternSource.cpp
)
add_library(pivision::capture ALIAS pivision_capture)

target_include_directories(pivision_capture
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_capture
    PUBLIC  opencv_core
    PRIVATE opencv_imgproc
)

