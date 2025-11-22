#include "DrawUtils.h"

#include "Application/AppWindow.h"
#include "Foundation/Constants/SpecialCharacters.h"

// draw a border around the current view area
void DrawUtils_Border(int x, int y, int width, int height, View &view) {
  GUITextProperties props;

  char line[SCREEN_WIDTH + 1];

  // upper border
  memset(line, char_border_single_horizontal, width);
  line[0] = char_border_single_topLeft;
  line[width] = char_border_single_topRight;
  line[width + 1] = 0;
  view.DrawString(x, y, line, props);

  // lower border
  line[0] = char_border_single_bottomLeft;
  line[width] = char_border_single_bottomRight;
  view.DrawString(x, y + height, line, props);

  // left and right borders
  for (int i = 1; i < height; i++) {
    view.DrawString(x, y + i, char_border_single_vertical_s, props);
    view.DrawString(x + width, y + i, char_border_single_vertical_s, props);
  }
}