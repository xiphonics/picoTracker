add_library(application_views_baseclasses
  I_Action.cpp
  ModalView.cpp
  UIActionField.cpp
  UIBigHexVarField.cpp
  UIField.cpp
  UIIntField.cpp
  UIIntVarField.cpp
  UIIntVarOffField.cpp
  UINoteVarField.cpp
  UISortedVarList.cpp
  UIStaticField.cpp
  UITempoField.cpp
  UISwatchField.cpp
  UIBitmaskVarField.cpp
  View.cpp
  ViewEvent.cpp
  UIBitmapField.cpp
)

target_link_libraries(application_views_baseclasses PUBLIC
  application_utils
  system_console
  platform_display
)

target_include_directories(application_views_baseclasses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
