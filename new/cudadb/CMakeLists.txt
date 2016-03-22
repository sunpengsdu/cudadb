cmake_minimum_required (VERSION 2.8)
project (MOG)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_VERBOSE_MAKEFILE ON)

FIND_PACKAGE(CUDA REQUIRED)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    message("The compiler ${CMAKE_CXX_COMPILER} support c++11")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -std=c++11")

set(GENCODE -gencode=arch=compute_30,code=sm_30 -gencode=arch=compute_35,code=compute_35)
set(GENCODE ${GENCODE} -gencode=arch=compute_20,code=sm_20)
set(CUDA_NVCC_FLAGS_DEBUG "-g -G")
set(CUDA_NVCC_FLAGS_RELEASE "-O2")

set(LINK_DIR /lib /usr/lib /usr/local/lib ${CMAKE_SOURCE_DIR}/src/tools/lib)

set(CMAKE_CXX_COMPILER "g++")
INCLUDE_DIRECTORIES(/usr/local/include)

FIND_LIBRARY(libyaml-cpp NAMES "yaml-cpp" PATHS ${LINK_DIR})
IF(NOT libyaml-cpp)
MESSAGE(FATAL_ERROR "libyaml-cpp not found")
ENDIF(NOT libyaml-cpp)
MESSAGE(--${libyaml-cpp})

FIND_LIBRARY(libglog NAMES "glog" PATHS ${LINK_DIR})
IF(NOT libglog)
MESSAGE(FATAL_ERROR "libglog not found")
ENDIF(NOT libglog)
MESSAGE(--${libglog})

FIND_LIBRARY(libleveldb NAMES "leveldb" PATHS ${LINK_DIR})
IF(NOT libleveldb)
MESSAGE(FATAL_ERROR "libleveldb not found")
ENDIF(NOT libleveldb)
MESSAGE(--${libleveldb})

FIND_LIBRARY(libcuda NAMES "cuda" PATHS ${LINK_DIR})
IF(NOT libcuda)
MESSAGE(FATAL_ERROR "libcuda not found")
ENDIF(NOT libcuda)
MESSAGE(--${libcuda})

FIND_PACKAGE( Boost 1.5 COMPONENTS system filesystem thread REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_subdirectory(src)
