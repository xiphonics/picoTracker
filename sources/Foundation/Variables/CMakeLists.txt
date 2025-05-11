add_library(foundation_variables
  Variable.cpp
  VariableContainer.cpp
  WatchedVariable.cpp
)

target_include_directories(foundation_variables PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
