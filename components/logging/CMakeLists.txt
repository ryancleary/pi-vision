add_library(pivision_logging STATIC
    include/pivision/logging/Logging.h
    private/Utilities.h
    src/Logging.cpp
    src/Utilities.cpp
)
add_library(pivision::logging ALIAS pivision_logging)

target_include_directories(pivision_logging
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_logging
    PUBLIC Qt6::Core
)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
