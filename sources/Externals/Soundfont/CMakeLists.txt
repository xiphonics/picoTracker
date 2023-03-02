add_library(soundfont
  DATATYPE.H
  EMUERRS.H
  ENAB.H ENAB.CPP
#  ENABDRV.CPP
  HYDRA.H HYDRA.CPP
  OMEGA.H OMEGA.CPP
  RIFF.H RIFF.CPP
  SFDATA.H
  SFDETECT.H SFDETECT.CPP
  SFENUM.H
  SFLOOKUP.H SFLOOKUP.CPP
  SFNAV.H SFNAV.CPP
  SFREADER.H SFREADER.CPP
  WIN_MEM.H WIN_MEM.CPP
)

target_link_libraries(soundfont PUBLIC application_utils)

target_include_directories(soundfont PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR})

set_target_properties(soundfont PROPERTIES COMPILE_DEFINITIONS "CPP_MEMORY")
