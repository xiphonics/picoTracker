add_library(foundation
  I_Iterator.h
  I_List.h
  IteratorPtr.h
  Observable.h Observable.cpp
  SingletonRegistry.h SingletonRegistry.cpp
  T_Factory.h
  T_SimpleList.h
  T_Singleton.h
  T_Stack.h
)

target_link_libraries(foundation PUBLIC system_console
)

target_include_directories(foundation PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(Variables)
add_subdirectory(Services)
