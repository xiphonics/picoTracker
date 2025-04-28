#ifndef _PERSISTENCE_CONSTANTS_H_
#define _PERSISTENCE_CONSTANTS_H_

#define MAX_PROJECT_NAME_LENGTH 16
#define MAX_INSTRUMENT_FILENAME_LENGTH 24
#define MAX_THEME_NAME_LENGTH 16
#define MAX_THEME_EXPORT_PATH_LENGTH                                           \
  (MAX_THEME_NAME_LENGTH + strlen(THEMES_DIR) + 1 +                            \
   strlen(THEME_FILE_EXTENSION))
// accounts for .pti extension
#define MAX_INSTRUMENT_NAME_LENGTH (MAX_INSTRUMENT_FILENAME_LENGTH - 4)

#define PROJECTS_DIR "/projects"
#define PROJECT_SAMPLES_DIR "samples"
#define SAMPLES_LIB_DIR "/samples"
#define INSTRUMENTS_DIR "/instruments"
#define RENDERS_DIR "/renders"
#define THEMES_DIR "/themes"
#define INSTRUMENT_FILE_EXTENSION ".pti"
#define THEME_FILE_EXTENSION ".ptt"

#endif // _PERSISTENCE_CONSTANTS_H_