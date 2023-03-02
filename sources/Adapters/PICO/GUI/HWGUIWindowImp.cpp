#include "HWGUIWindowImp.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include <string.h>
// #include "Application/Utils/assert.h"
#include "Application/Utils/char.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include "Adapters/PICO/Utils/utils.h"
#include <string>

// #define to_rgb565(color) ((color._r & 0b11111000) << 8) | ((color._g &
// 0b11111100) << 3) | (color._b >> 3)

HWGUIWindowImp *instance_;

HWGUIWindowImp::HWGUIWindowImp(GUICreateWindowParams &p) {
  mode0_init();
  instance_ = this;
};

HWGUIWindowImp::~HWGUIWindowImp() {}

void HWGUIWindowImp::DrawChar(const char c, GUIPoint &pos,
                              GUITextProperties &p) {
  //  Trace::Debug("Draw char \"%c\" at pos x:%ld (%ld), y:%ld (%ld) - invert: %d", c, pos._x, pos._x / 8, pos._y, pos._y / 8, p.invert_);

  uint8_t x = pos._x / 8;
  uint8_t y = pos._y / 8;
  mode0_set_cursor(x, y);
  mode0_putc(c, p.invert_);
}

void HWGUIWindowImp::DrawString(const char *string, GUIPoint &pos,
                                GUITextProperties &p, bool overlay) {
  Trace::Debug("draw string");
  mode0_set_cursor(pos._x, pos._y);
  mode0_print(string, p.invert_);
};

void HWGUIWindowImp::DrawRect(GUIRect &r) {
  Trace::Debug("GUI DrawRect call");
};

void HWGUIWindowImp::Clear(GUIColor &c, bool overlay) {
  mode0_color_t backgroundColor = GetColor(c);
  mode0_set_background(backgroundColor);
  mode0_clear(backgroundColor);
};

void HWGUIWindowImp::ClearRect(GUIRect &r) {
  Trace::Debug("GUI ClearRect call");
};

mode0_color_t HWGUIWindowImp::GetColor(GUIColor &c) {
  // TODO: This is hacky AF but works for now
  if (c._r == 119 && c._g == 107 && c._b == 86) {
    return MODE0_BROWN;
  } else if (c._r == 241 && c._g == 241 && c._b == 150) {
    return MODE0_YELLOW;
  } else if (c._r == 142 && c._g == 160 && c._b == 74) {
    return MODE0_BLUE;
  } else if (c._r == 168 && c._g == 22 && c._b == 22) {
    return MODE0_RED;
  } else {
    // This shouldn't appear, seems like there are 4 colors used only
    Trace::Debug("Selected color UNKNOWN!!!");
    return MODE0_BLUSH;
  }
}
void HWGUIWindowImp::SetColor(GUIColor &c) {
  mode0_set_foreground(GetColor(c));
};

void HWGUIWindowImp::Lock() {};

void HWGUIWindowImp::Unlock() {};

void HWGUIWindowImp::Flush() {
  mode0_draw_changed();
};

void HWGUIWindowImp::Invalidate() {
  PICOEvent *event = new PICOEvent();
  event->type_ = PICO_REDRAW;
  PICOEventQueue::GetInstance()->Push(*event);
};

void HWGUIWindowImp::PushEvent(GUIEvent &event) {
  Trace::Debug("GUI PushEvent");
};

GUIRect HWGUIWindowImp::GetRect() {
  Trace::Debug("GUI GetRect");
  return GUIRect(0, 0, 320, 240);
}

void HWGUIWindowImp::ProcessEvent(PICOEvent &event) {
  switch (event.type_) {
  case PICO_REDRAW:
    //        instance_->currentBuffer_^=0x01 ;
    instance_->_window->Update();
    //        gp_setFramebuffer(instance_->framebuffer_[instance_->currentBuffer_],1);
    break;
  case PICO_QUIT:
    System *system = System::GetInstance();
    unsigned long now = system->GetClock();
    GUIEvent event(EKT_ESCAPE, ET_KEYDOWN, now, 0, 0, 0);
    instance_->_window->DispatchEvent(event);
    break;
  }
}

static GUIEventPadButtonType eventMapping[10] = {
    EPBT_LEFT, EPBT_DOWN, EPBT_RIGHT, EPBT_UP,    EPBT_L,
    EPBT_B,    EPBT_A,    EPBT_R,     EPBT_START, EPBT_SELECT};

void HWGUIWindowImp::ProcessButtonChange(uint16_t changeMask,
                                         uint16_t buttonMask) {
  int e = 1;
  System *system = System::GetInstance();
  unsigned long now = system->GetClock();
  for (int i = 0; i < 10; i++) {
    if (changeMask & e) {
      GUIEventType type = (buttonMask & e) ? ET_PADBUTTONDOWN : ET_PADBUTTONUP;

      GUIEvent event(eventMapping[i], type, now, 0, 0, 0);
      instance_->_window->DispatchEvent(event);
    }
    e = e << 1;
  }
}
