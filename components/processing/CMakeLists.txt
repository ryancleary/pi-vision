add_library(pivision_processing STATIC
    include/pivision/processing/Stage.h
    include/pivision/processing/StageChain.h
    include/pivision/processing/StageFactory.h
    private/BuiltinStages.h
    src/BuiltinStages.cpp
    src/Stage.cpp
    src/StageChain.cpp
    src/StageFactory.cpp
)
add_library(pivision::processing ALIAS pivision_processing)

target_include_directories(pivision_processing
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_processing
    PUBLIC  opencv_core
    PRIVATE opencv_imgproc
)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
