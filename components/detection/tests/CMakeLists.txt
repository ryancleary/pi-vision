pivision_add_tests(detection
    SOURCES
        ModelConfigGTest.cpp
        ObjectDetectorGTest.cpp
        UtilsGTest.cpp
    LIBRARIES
        opencv_dnn
        opencv_imgcodecs
)

# The tests test the private ObjectDetector class directly.
target_include_directories(pivision_detection_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../private)

# The model descriptions (in git) and the models (from scripts/fetch-assets.sh).
# Tests that need a model skip themselves if it hasn't been fetched.
target_compile_definitions(pivision_detection_tests PRIVATE
    PIVISION_MODEL_DIR="${PROJECT_SOURCE_DIR}/assets/models"
    PIVISION_MEDIA_DIR="${PROJECT_SOURCE_DIR}/assets/media"
)
