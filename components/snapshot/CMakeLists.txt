add_library(pivision_snapshot STATIC
    include/pivision/snapshot/Export.h
    include/pivision/snapshot/SnapshotStore.h
    private/Utils.h
    src/Export.cpp
    src/SnapshotStore.cpp
    src/Utils.cpp
)
add_library(pivision::snapshot ALIAS pivision_snapshot)

target_include_directories(pivision_snapshot
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(pivision_snapshot
    PUBLIC  Qt6::Core Qt6::Gui
    PRIVATE pivision::logging pivision::utils
)

if(PIVISION_BUILD_TESTS)
    add_subdirectory(tests)
endif()
