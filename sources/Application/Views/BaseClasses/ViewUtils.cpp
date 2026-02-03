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
