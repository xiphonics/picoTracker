add_library(crsid
  SID.h SID.cpp
  SIDfilter.h
  SIDwaves.h

)

#target_link_libraries(crsid PUBLIC xxx
#)

target_include_directories(crsid PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
