add_library(pivision_system STATIC
    include/pivision/system/SystemProbe.h
    private/Utils.h
    src/SystemProbe.cpp
    src/Utils.cpp
)
add_library(pivision::system ALIAS pivision_system)

target_include_directories(pivision_system
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
