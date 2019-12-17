add_subdirectory(lib)

add_executable(unlegato unlegato.cpp)
target_link_libraries(unlegato midi)

add_executable(chordify chordify.cpp)
target_link_libraries(chordify midi)
