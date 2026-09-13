# Small helpers used by more than one component. Helpers only one component
# needs live in that component's private/Utilities.h instead.
add_library(pivision_utils STATIC
    include/pivision/utils/Files.h
    include/pivision/utils/Images.h
    include/pivision/utils/Strings.h
    src/Files.cpp
    src/Images.cpp
    src/Strings.cpp
)
add_library(pivision::utils ALIAS pivision_utils)

target_include_directories(pivision_utils
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pivision_utils
    PUBLIC Qt6::Core Qt6::Gui opencv_core
)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
