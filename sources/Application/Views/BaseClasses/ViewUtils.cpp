#include "ViewUtils.h"

#include "Application/AppWindow.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include <string.h>

#define LABEL_COLOR CD_NORMAL
#define VALUE_COLOR CD_INFO

void DrawLabeledField(GUIWindow &w, GUIPoint position, char *buffer) {
  GUITextProperties props;

  char *colon = strchr(buffer, ':');
  if (colon) {
    *colon = '\0';
    // color used for the field label text:
    ((AppWindow &)w).SetColor(LABEL_COLOR);
    w.DrawString(buffer, position, props);
    position._x += strlen(buffer);
    *colon = ':';
    ((AppWindow &)w).SetColor(VALUE_COLOR);
    w.DrawString(colon, position, props);
  } else {
    ((AppWindow &)w).SetColor(CD_NORMAL);
    w.DrawString(buffer, position, props);
  }
}
