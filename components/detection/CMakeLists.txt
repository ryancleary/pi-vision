add_library(pivision_detection STATIC
    include/pivision/detection/Detection.h
    include/pivision/detection/ModelConfig.h
    private/ObjectDetector.h
    private/Utils.h
    src/ModelConfig.cpp
    src/ObjectDetector.cpp
    src/Utils.cpp
)
add_library(pivision::detection ALIAS pivision_detection)

target_include_directories(pivision_detection
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_detection
    PUBLIC  opencv_core
    PRIVATE opencv_dnn
)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
