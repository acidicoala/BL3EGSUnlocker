cmake_minimum_required(VERSION 3.22)

project(BL3EGSUnlocker VERSION 1.1.0)

include(KoalaBox/cmake/KoalaBox.cmake)

add_subdirectory(KoalaBox EXCLUDE_FROM_ALL)

configure_version_resource("Borderlands 3 EGS DLC Unlocker")

configure_build_config()

set(BL3EGSUnlocker_SOURCES src/main.cpp src/unlocker/unlocker.cpp src/unlocker/unlocker.hpp)

add_library(BL3EGSUnlocker SHARED ${BL3EGSUnlocker_SOURCES} ${VERSION_RESOURCE})

configure_output_name(Unlocker)

configure_include_directories()

target_link_libraries(BL3EGSUnlocker PRIVATE KoalaBox)
