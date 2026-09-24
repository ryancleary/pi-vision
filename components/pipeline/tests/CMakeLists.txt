pivision_add_tests(pipeline
    SOURCES
        FrameMetricsGTest.cpp
        LatestFrameBufferGTest.cpp
        PipelineGTest.cpp
        StageConfigGTest.cpp
    LIBRARIES
        pivision::testsupport
        Qt6::Test
)

# The detection model (in git: the description; fetched: the .onnx). Tests
# that need the model skip themselves if it hasn't been fetched.
target_compile_definitions(pivision_pipeline_tests PRIVATE
    PIVISION_MODEL_DIR="${PROJECT_SOURCE_DIR}/assets/models"
)
