
cmake_minimum_required(VERSION 2.8)
project(reaper)

## Version ##

set(REAPER_VERSION "v11")

## Options ##

option(CPU_MINING_ONLY "Do not compile the GPU mining part" OFF)

## Global config ##

configure_file(
    "${PROJECT_SOURCE_DIR}/CMakeConf.h.in"
    "${PROJECT_BINARY_DIR}/CMakeConf.h"
)

## Source files ##

set(SOURCES
    App.cpp
    AppOpenCL.cpp
    Blake512.cpp
    CPUMiner.cpp
    Config.cpp
    Curl.cpp
    json_reader.cpp
    json_value.cpp
    json_writer.cpp
    main.cpp
    RSHash.cpp
	ServerSettings.cpp
    SHA256.cpp
    Util.cpp
)

set(HEADERS 
	App.h
	AppOpenCL.h
	Blake512.h
	CPUMiner.h
	Config.h
	Curl.h
	Global.h
	RSHash.h
	ServerSettings.h
	SHA256.h
	Util.h
)

## Targets ##

add_executable(reaper ${SOURCES} ${HEADERS})

include_directories(${PROJECT_BINARY_DIR})

set_target_properties(reaper PROPERTIES COMPILE_FLAGS -O2)

## OpenCL detection ##
if(NOT CPU_MINING_ONLY)
    
    find_library(OPENCL_LIBRARY OpenCL)
    
    if(APPLE)
        find_path(OPENCL_INCLUDE_DIR OpenCL/cl.h)
    else(APPLE)
        find_path(OPENCL_INCLUDE_DIR CL/cl.h)
    endif(APPLE)
    
    mark_as_advanced(OPENCL_INCLUDE_DIR OPENCL_LIBRARY)
    
    include_directories(${OPENCL_INCLUDE_DIR})
    
endif(NOT CPU_MINING_ONLY)

## Detection of the rest of the libs ##

find_library(PTHREAD_LIBRARY pthread)
find_path(PTHREAD_INCLUDE_DIR NAMES pthread.h)

find_library(CURL_LIBRARY curl)
find_path(CURL_INCLUDE_DIR NAMES curl.h PATH_SUFFIXES curl)

include_directories(${PTHREAD_INCLUDE_DIR} ${CURL_INCLUDE_DIR})
link_directories(${PTHREAD_LIBRARY} ${CURL_LIBRARY})

## Library configuration ##

if(CPU_MINING_ONLY)
    target_link_libraries(reaper ${PTHREAD_LIBRARY} ${CURL_LIBRARY})
else(CPU_MINING_ONLY)
    target_link_libraries(reaper ${PTHREAD_LIBRARY} ${CURL_LIBRARY} ${OPENCL_LIBRARY})
endif(CPU_MINING_ONLY)
