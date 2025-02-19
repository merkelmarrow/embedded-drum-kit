cmake_minimum_required (VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# this must be done before project()
# must set PICO_SDK_PATH in your environment variables
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project ("drum-kit" C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# used by pico_set_program_url
set(PICO_APPS_PATH ${PROJECT_SOURCE_DIR})

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Specify the location where picotoolConfig.cmake is installed
set(picotool_DIR "C:/Program Files (x86)/picotool/lib/cmake/picotool")

# Find and include picotool
find_package(picotool REQUIRED)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

add_executable(drum-kit)

# Specify the source files to be compiled.
target_sources(drum-kit PRIVATE drum-kit.cpp)

# Pull in commonly used features.
target_link_libraries(drum-kit PRIVATE pico_stdlib)

# Create map/bin/hex file etc.
pico_add_extra_outputs(drum-kit)
