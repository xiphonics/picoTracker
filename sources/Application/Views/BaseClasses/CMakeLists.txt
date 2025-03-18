add_library(application_views_baseclasses
  I_Action.h I_Action.cpp
  ModalView.h ModalView.cpp
  UIActionField.h UIActionField.cpp
  UIBigHexVarField.h UIBigHexVarField.cpp
  UIField.h UIField.cpp
  UIIntField.h UIIntField.cpp
  UIIntVarField.h UIIntVarField.cpp
  UIIntVarOffField.h UIIntVarOffField.cpp
  UINoteVarField.h UINoteVarField.cpp
  UISortedVarList.h UISortedVarList.cpp
  UIStaticField.h UIStaticField.cpp
  UITempoField.h UITempoField.cpp
  UITextField.h
  UISwatchField.h UISwatchField.cpp
  UIBitmaskVarField.h UIBitmaskVarField.cpp
  View.h View.cpp
  ViewEvent.h ViewEvent.cpp
)

target_link_libraries(application_views_baseclasses PUBLIC pico_stdlib
                                          PUBLIC sdfat
                                          PUBLIC platform_sdcard
                                          PUBLIC application_utils
                                          PUBLIC system_console
)

target_include_directories(application_views_baseclasses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})
