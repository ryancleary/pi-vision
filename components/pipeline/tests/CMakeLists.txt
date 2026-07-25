pivision_add_tests(pipeline
    SOURCES
        LatestFrameBufferGTest.cpp
        PipelineGTest.cpp
    LIBRARIES
        pivision::testsupport
        Qt6::Test
)

