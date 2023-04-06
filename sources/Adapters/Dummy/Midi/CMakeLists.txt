add_library(dummy_midi
  DummyMidi.h DummyMidi.cpp
)

#target_link_libraries(dummy_midi PUBLIC xxx
#)

target_include_directories(dummy_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

