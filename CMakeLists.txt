cmake_minimum_required(VERSION 3.0)
project(UNLEGATO)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNO_DEBUG")

add_subdirectory(lib)

add_executable(unlegato unlegato.cpp)
target_link_libraries(unlegato midi)

add_executable(chordify chordify.cpp)
target_link_libraries(chordify midi)

add_executable(microGranny microGranny.cpp)
target_link_libraries(microGranny midi)
