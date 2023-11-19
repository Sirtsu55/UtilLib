# CMakeLists.txt for the UtilLib project

cmake_minimum_required(VERSION 3.9)

project(UtilLib VERSION 1.0.0 DESCRIPTION "UtilLib")

# C++ 20
set(CMAKE_CXX_STANDARD 20)


# Add Interface Library
add_library(UtilLib INTERFACE)

# Add include directories
target_include_directories(UtilLib INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>)