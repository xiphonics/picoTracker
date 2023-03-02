add_library(tiny_xml
  Tiny2NosStub.h Tiny2NosStub.cpp
  tinystr.h tinystr.cpp
  tinyxml.h tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
  )

target_link_libraries(tiny_xml PUBLIC system_filesystem
)

target_include_directories(tiny_xml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
