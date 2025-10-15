#include "ViewUtils.h"

#include "Application/AppWindow.h"
#include "UIFramework/Interfaces/I_GUIGraphics.h"
#include <string.h>

void DrawColoredField(GUIWindow &w, GUIPoint position, char *buffer) {
  GUITextProperties props;

  char *colon = strchr(buffer, ':');
  if (colon) {
    *colon = '\0';
    // color used for the field label text:
    ((AppWindow &)w).SetColor(CD_INFO);
    w.DrawString(buffer, position, props);
    position._x += strlen(buffer);
    *colon = ':';
    ((AppWindow &)w).SetColor(CD_NORMAL);
    w.DrawString(colon, position, props);
  } else {
    ((AppWindow &)w).SetColor(CD_NORMAL);
    w.DrawString(buffer, position, props);
  }
}
