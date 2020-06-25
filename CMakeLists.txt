cmake_minimum_required(VERSION 3.1)

#set project name and language
project(vf-net2 LANGUAGES CXX)

#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#C++ 11 Flag
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Set project Version
set(VfNet2_VERSION_MAJOR "1")
set(VfNet2_VERSION_MINOR "3")
set(VfNet2_VERSION_PATCH "0")

set(PROJECT_VERSION "${VfNet2_VERSION_MAJOR}.${VfNet2_VERSION_MINOR}.${VfNet2_VERSION_PATCH}")

#Adding cmake functions like feature_summary()
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


#sum up project Files
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create library 
add_library(VfNet2 SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )

#link dependencies
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

#set target Version
set_target_properties(VfNet2 PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(VfNet2 PROPERTIES SOVERSION ${VfNet2_VERSION_MAJOR})


#generate export header
include(GenerateExportHeader)
generate_export_header(VfNet2)

#install export header
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

# install library
install(TARGETS VfNet2
    EXPORT VfNet2Export
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    VfNet2ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *Config.cmake
configure_file(VfNet2Config.cmake.in VfNet2Config.cmake @ONLY)


# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/VfNet2Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VfNet2ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfNet2
    )

# install targets cmake-files
install(EXPORT VfNet2Export
    FILE VfNet2Targets.cmake
    NAMESPACE VeinMeta::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VfNet2
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/VfNet2
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)



