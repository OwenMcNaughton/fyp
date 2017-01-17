cmake_minimum_required(VERSION 3.5)

project (Circ)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -std=c++11 -w")

file(GLOB_RECURSE App_SOURCES "./*.cc")
file(GLOB_RECURSE App_HEADERS "./*.hh")

set (App_INCLUDE_DIRS "")
foreach (_headerFile ${App_HEADERS})
  get_filename_component(_dir ${_headerFile} PATH)
  list (APPEND App_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES App_INCLUDE_DIRS)

include_directories(${App_INCLUDE_DIRS})
add_executable (main ${App_SOURCES})
TARGET_LINK_LIBRARIES(main pthread)
