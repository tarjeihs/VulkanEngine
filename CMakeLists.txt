cmake_minimum_required(VERSION 3.16)
project(Rocket)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "Please set the VCPKG_ROOT environment variable to point to your vcpkg installation. Use command: 'set VCPKG_ROOT=C:\\vcpkg'")
endif()

# Set VCPKG_ROOT and the toolchain file
set(VCPKG_ROOT $ENV{VCPKG_ROOT})
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Specify the vcpkg target triplet
set(VCPKG_TARGET_TRIPLET "x64-windows")

# Use only Debug, Release, and Dist configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)

# Enable parallel compilation
add_compile_options(/MP)

# Output directories
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/Binaries)
set(INTERMEDIATE_DIR ${CMAKE_BINARY_DIR}/Intermediate)

# Collect all source files for the Engine project
file(GLOB_RECURSE ENGINE_SOURCES
    Engine/Source/*.cpp
    Engine/Source/*.h
)

# Find Vulkan, glfw, and glm
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Engine project
add_library(Engine STATIC ${ENGINE_SOURCES})

target_include_directories(Engine PRIVATE
    Engine/Source
    ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(Engine PRIVATE
    glfw
    glm::glm
    Vulkan::Vulkan
)

target_compile_definitions(Engine PRIVATE
    _CRT_SECURE_NO_WARNINGS
    GLFW_INCLUDE_VULKAN
)

target_compile_options(Engine PRIVATE /std:c++17)

set_target_properties(Engine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/$<CONFIG>/Engine
    RUNTIME_OUTPUT_DIRECTORY ${INTERMEDIATE_DIR}/$<CONFIG>/Engine
    PCH_FILE EnginePCH.h
    PCH_SOURCE Engine/Source/EnginePCH.cpp
)

if(WIN32)
    target_compile_definitions(Engine PRIVATE
        MOD_PLATFORM_WINDOWS
        MOD_BUILD_DLL
    )
endif()

# Collect all source files for the Editor project
file(GLOB_RECURSE EDITOR_SOURCES
    Editor/Source/*.cpp
    Editor/Source/*.h
)

# Editor project
add_executable(Editor ${EDITOR_SOURCES})

target_include_directories(Editor PRIVATE
    Editor/Source
    Editor/Vendor
    Engine/Source
    Engine/Vendor
    ${Vulkan_INCLUDE_DIR}
)

target_link_libraries(Editor PRIVATE Engine
    Vulkan::Vulkan
)

target_compile_definitions(Editor PRIVATE
    _CRT_SECURE_NO_WARNINGS
)

target_compile_options(Editor PRIVATE /std:c++17)

set_target_properties(Editor PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/$<CONFIG>/Editor
    RUNTIME_OUTPUT_DIRECTORY ${INTERMEDIATE_DIR}/$<CONFIG>/Editor
)

if(WIN32)
    target_compile_definitions(Editor PRIVATE
        WB_PLATFORM_WINDOWS
    )
endif()

# Configuration-specific flags
set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od /RTC1")
set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_DIST "/MD /O2 /Ob2 /DNDEBUG")

# Configuration-specific linker flags
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "")
set(CMAKE_EXE_LINKER_FLAGS_DIST "")

# Configuration-specific defines
foreach(CONFIG_TYPE IN ITEMS Debug Release Dist)
    string(TOUPPER ${CONFIG_TYPE} CONFIG_NAME_UPPER)
    if(${CONFIG_TYPE} STREQUAL "Debug")
        target_compile_definitions(Engine PRIVATE MOD_DEBUG)
        target_compile_definitions(Editor PRIVATE MOD_DEBUG)
    elseif(${CONFIG_TYPE} STREQUAL "Release")
        target_compile_definitions(Engine PRIVATE MOD_RELEASE)
        target_compile_definitions(Editor PRIVATE MOD_RELEASE)
    elseif(${CONFIG_TYPE} STREQUAL "Dist")
        target_compile_definitions(Engine PRIVATE MOD_DIST)
        target_compile_definitions(Editor PRIVATE MOD_DIST)
    endif()
endforeach()
