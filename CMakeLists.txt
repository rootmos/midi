cmake_minimum_required(VERSION 3.0)
project(MIDI)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNO_DEBUG")

add_subdirectory(src)
