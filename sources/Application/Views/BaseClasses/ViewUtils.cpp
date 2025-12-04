#include "ViewUtils.h"

#include "Application/AppWindow.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include <string.h>

#define LABEL_COLOR CD_NORMAL
#define VALUE_COLOR CD_EMPHASIS

void DrawLabeledField(GUIWindow &w, GUIPoint position, char *buffer) {
  GUITextProperties props;

  char *colon = strchr(buffer, ':');
  if (colon) {
    char *cut = colon + 1;
    char value = *cut;
    *cut = '\0';

    ((AppWindow &)w).SetColor(LABEL_COLOR);
    w.DrawString(buffer, position, props);
    position._x += strlen(buffer);

    *cut = value; // restore char
    ((AppWindow &)w).SetColor(VALUE_COLOR);
    w.DrawString(cut, position, props);
  } else {
    // Fields that don't have a colon are all value
    ((AppWindow &)w).SetColor(VALUE_COLOR);
    w.DrawString(buffer, position, props);
  }
}

bool goProjectSamplesDir(ViewData *viewData_) {
  auto fs = FileSystem::GetInstance();
  fs->chdir(PROJECTS_DIR);
  // Then, navigate into the current project's directory
  if (viewData_ && viewData_->project_) {
    char projectName[MAX_PROJECT_NAME_LENGTH + 1];
    viewData_->project_->GetProjectName(projectName);

    if (fs->chdir(projectName)) {
      // Finally, navigate into the samples subdirectory
      fs->chdir(PROJECT_SAMPLES_DIR);
    } else {
      Trace::Error("SampleEditorView: Failed to chdir to project dir: %s",
                   projectName);
      // It's good practice to return to the root to avoid being in an unknown
      // state
      fs->chdir("/");
      return false; // Abort if we can't find the project directory
    }
  } else {
    Trace::Error(
        "SampleEditorView: No project data available to find samples dir.");
    fs->chdir("/");
    return false; // Abort if project data is missing
  }
  return true;
}
