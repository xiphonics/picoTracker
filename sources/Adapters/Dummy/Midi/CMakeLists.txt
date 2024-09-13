add_library(dummy_midi
  DummyMidi.h DummyMidi.cpp
)

#target_link_libraries(dummy_midi PUBLIC xxx
#)

target_include_directories(dummy_midi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(dummy_midi PUBLIC 
  PRIVATE sdfat # need this due transitive dep on XML lib coming from the Config class dep
)

include_directories(${PROJECT_SOURCE_DIR})

