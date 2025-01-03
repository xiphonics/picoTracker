add_library(opal
opal.h opal.cpp
)

target_link_libraries(opal 
    PUBLIC application_utils
    PUBLIC hardware_flash
)

target_include_directories(opal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
