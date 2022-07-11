cmake_minimum_required(VERSION 3.1)

project(vf-net2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VfNet2_VERSION_MAJOR "1")
set(VfNet2_VERSION_MINOR "3")
set(VfNet2_VERSION_PATCH "0")

set(PROJECT_VERSION "${VfNet2_VERSION_MAJOR}.${VfNet2_VERSION_MINOR}.${VfNet2_VERSION_PATCH}")

include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Core Network CONFIG REQUIRED)
find_package(VfTcp REQUIRED)
find_package(VfHelpers REQUIRED)
find_package(VfComponent REQUIRED)
find_package(VfEvent REQUIRED)
find_package(Flatbuffers REQUIRED)

#generate ecs_schema_generated from ecs_schema.fbs with flatbuffer 
#ToDo solve make ecs_schema.fbs dependency (but it works already)
execute_process(COMMAND flatc --cpp ${PROJECT_SOURCE_DIR}/ecs_schema.fbs
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )


file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

add_library(VfNet2 SHARED
    ${SOURCES}
    ${HEADER}
    )

target_link_libraries(VfNet2
    PRIVATE
    Qt5::Core
    Qt5::Network
    flatbuffers
    VeinMeta::VfHelpers
    VeinMeta::VfComponent
    VeinMeta::VfTcp
    VeinMeta::VfEvent

    )

set_target_properties(VfNet2 PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(VfNet2 PROPERTIES SOVERSION ${VfNet2_VERSION_MAJOR})

include(GenerateExportHeader)
generate_export_header(VfNet2)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vfnet2_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfNet2
    )

# announce headers - target perspective
target_include_directories(VfNet2
    PUBLIC
    $<INSTALL_INTERFACE:include/VfNet2>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

install(TARGETS VfNet2
    EXPORT VfNet2Export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    VfNet2ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
    )

configure_file(VfNet2Config.cmake.in VfNet2Config.cmake @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VfNet2Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VfNet2ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfNet2
    )

install(EXPORT VfNet2Export
    FILE VfNet2Targets.cmake
    NAMESPACE VeinMeta::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfNet2
    )

install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfNet2
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

