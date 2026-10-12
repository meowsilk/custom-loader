cmake_minimum_required(VERSION 3.25)

project(loader_gg VERSION 0.1.0 LANGUAGES C CXX RC)

if(NOT MSVC)
    message(FATAL_ERROR "loader.gg requires the MSVC toolchain on Windows")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    set(loader_tests_available ON)
else()
    set(loader_tests_available OFF)
endif()
option(LOADER_BUILD_TESTS "Build the unit tests" ${loader_tests_available})
option(LOADER_ENABLE_LTO "Link-time code generation for Release builds" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(external)
add_subdirectory(shaders)
add_subdirectory(assets)
add_subdirectory(src)

if(LOADER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT loader_gg)
