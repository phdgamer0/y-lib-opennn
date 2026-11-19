cmake_minimum_required(VERSION 3.16)
project(y-lib-opennn)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(FetchContent)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(OpenNN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OpenNN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OpenNN_BUILD_BLANK OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    opennn
    # pinned: OpenNN master dropped opennn/opennn.h (2026-04); v8.0.1 is the last release with it
    URL https://github.com/Artelnics/opennn/archive/refs/tags/v8.0.1.zip
)
FetchContent_MakeAvailable(opennn)
FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
)
FetchContent_MakeAvailable(eigen)
# y-- source tree to compile against; yenv passes its installed toolchain
set(YLANG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../y---lang" CACHE PATH "y-- source directory")
add_library(y-opennn SHARED module.cpp)
# symbols come from the ymm process at load time
if(APPLE)
    target_link_options(y-opennn PRIVATE -undefined dynamic_lookup)
endif()
target_include_directories(y-opennn PRIVATE ${YLANG_DIR}/include ${YLANG_DIR}/include/ylang ${YLANG_DIR}/src ${opennn_SOURCE_DIR} ${eigen_SOURCE_DIR})
target_link_libraries(y-opennn PRIVATE opennn)
