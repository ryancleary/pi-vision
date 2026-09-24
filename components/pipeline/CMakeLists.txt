add_library(pivision_pipeline STATIC
    include/pivision/pipeline/DetectionResult.h
    include/pivision/pipeline/FrameMetrics.h
    include/pivision/pipeline/LatestFrameBuffer.h
    include/pivision/pipeline/Pipeline.h
    include/pivision/pipeline/StageConfig.h
    private/DetectionJob.h
    private/DetectionWorker.h
    private/FrameProcessor.h
    private/FrameProducer.h
    private/RawFrame.h
    private/Utils.h
    src/DetectionWorker.cpp
    src/FrameMetrics.cpp
    src/FrameProcessor.cpp
    src/FrameProducer.cpp
    src/Pipeline.cpp
    src/StageConfig.cpp
    src/Utils.cpp
)
add_library(pivision::pipeline ALIAS pivision_pipeline)

target_include_directories(pivision_pipeline
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_pipeline
    PUBLIC  pivision::capture pivision::processing pivision::detection Qt6::Core Qt6::Gui
    PRIVATE pivision::logging pivision::utils opencv_imgproc
)

# Lets QML modules in other components refer to Pipeline (see display).
qt_extract_metatypes(pivision_pipeline)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
