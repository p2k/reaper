
cmake_minimum_required(VERSION 2.8)
project(reaper)

## Version ##

set(REAPER_VERSION "v10")

## Options ##

option(CPU_MINING_ONLY "Do not compile the GPU mining part" OFF)
option(LONGPOLLING "Enable experimental long polling support" OFF)

if(LONGPOLLING)
    add_definitions(-DLONGPOLLING)
endif(LONGPOLLING)

## Global config ##

configure_file(
    "${PROJECT_SOURCE_DIR}/Global.h.in"
    "${PROJECT_BINARY_DIR}/Global.h"
)

## Source files ##

set(SOURCES
    App.cpp
    Blake512.cpp
    Config.cpp
    Curl.cpp
    json_reader.cpp
    json_value.cpp
    json_writer.cpp
    main.cpp
    AppOpenCL.cpp
    RSHash.cpp
    SHA256.cpp
    Util.cpp
    CPUMiner.cpp
)

## Targets ##

add_executable(reaper ${SOURCES})

set_target_properties(reaper PROPERTIES COMPILE_FLAGS -O2)

## OpenCL detection ##
if(NOT CPU_MINING_ONLY)
    
    find_library(OPENCL_LIBRARY OpenCL)
    
    find_path(OPENCL_INCLUDE_DIR NAMES cl.h PATH_SUFFIXES CL)
    
    mark_as_advanced(OPENCL_INCLUDE_DIR OPENCL_LIBRARY)
    
    include_directories(${OPENCL_INCLUDE_DIR})
    
endif(NOT CPU_MINING_ONLY)

## Library configuration ##

if(CPU_MINING_ONLY)
    add_definitions(-DCPU_MINING_ONLY)
    target_link_libraries(reaper pthread curl)
else(CPU_MINING_ONLY)
    target_link_libraries(reaper pthread curl ${OPENCL_LIBRARY})
endif(CPU_MINING_ONLY)
