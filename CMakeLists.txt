cmake_minimum_required(VERSION 3.5)

project(Circ)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -std=c++11 -w")

file(GLOB Lib_SOURCES "./src/*.cc")
file(GLOB Lib_HEADERS "./src/*.hh")

add_library(CircLib ${Lib_SOURCES} ${Lib_HEADERS})
add_executable(main "./src/main.cc")
TARGET_LINK_LIBRARIES(main CircLib pthread)
