find_package(ALSA REQUIRED)

add_library(midi STATIC midi.cpp)
set_property( TARGET midi APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )
target_link_libraries(midi ${ALSA_LIBRARY})
target_include_directories(midi PUBLIC include)
