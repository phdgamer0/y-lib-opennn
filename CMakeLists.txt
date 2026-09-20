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
    URL https://github.com/Artelnics/opennn/archive/refs/heads/master.zip
)
FetchContent_MakeAvailable(opennn)
FetchContent_Declare(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.zip
)
FetchContent_MakeAvailable(eigen)
add_library(y-opennn SHARED module.cpp)
target_include_directories(y-opennn PRIVATE ../y---lang/include ../y---lang/src ${opennn_SOURCE_DIR} ${eigen_SOURCE_DIR})
target_link_libraries(y-opennn PRIVATE opennn)
