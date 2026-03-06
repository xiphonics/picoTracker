add_library(application_utils
  MemoryPool.cpp
  HexBuffers.cpp
  char.cpp
  fixed.cpp
  stringutils.cpp 
)

target_link_libraries(application_utils PUBLIC
  etl
)

if(NOT ADV)
  target_link_libraries(application_utils PUBLIC
    pico_sync
  )
endif()

target_include_directories(application_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
