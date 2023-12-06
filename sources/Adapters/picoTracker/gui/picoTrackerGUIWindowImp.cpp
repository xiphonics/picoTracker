#include "picoTrackerGUIWindowImp.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include <string.h>
// #include "Application/Utils/assert.h"
#include "Adapters/picoTracker/utils/utils.h"
#include "Application/Utils/char.h"
#include "UIFramework/BasicDatas/GUIEvent.h"
#include <string>

#define to_rgb565(color) ((color._r & 0b11111000) << 8) | ((color._g & 0b11111100) << 3) | (color._b >> 3)

// classic picotracker mapping
static GUIEventPadButtonType eventMappingPico[10] = {
    EPBT_LEFT, // SW1
    EPBT_DOWN,  // SW2
    EPBT_RIGHT, // SW3
    EPBT_UP, // SW4 
    EPBT_L, // SW5
    EPBT_B, // SW6
    EPBT_A, // SW7
    EPBT_R, // SW8
    EPBT_START, // SW9
    EPBT_SELECT // No SW
};

// M8 style mapping
static GUIEventPadButtonType eventMappingM8[10] = {
    EPBT_LEFT, // SW1
    EPBT_DOWN,  // SW2
    EPBT_RIGHT, // SW3
    EPBT_UP, // SW4
    EPBT_R ,  // SW5
    EPBT_A,  // SW6
    EPBT_L,  // SW7
    EPBT_START, // SW8
    EPBT_B , // SW9
    EPBT_SELECT// No SW
};

static GUIEventPadButtonType *eventMapping = eventMappingPico;

picoTrackerGUIWindowImp *instance_;

picoTrackerGUIWindowImp::picoTrackerGUIWindowImp(GUICreateWindowParams &p) {
  mode0_init();
  instance_ = this;

  Config *config = Config::GetInstance();
  const char *keymapStyle = config->GetValue("KEYMAPSTYLE");
  if (strcasecmp("M8",keymapStyle) == 0) {
    eventMapping = eventMappingM8;
    Trace::Log("GUIWINDOWIMP", "Using M8 keymap");
  }
};

picoTrackerGUIWindowImp::~picoTrackerGUIWindowImp() {}

void picoTrackerGUIWindowImp::DrawChar(const char c, GUIPoint &pos,
                                       GUITextProperties &p) {
  //  Trace::Debug("Draw char \"%c\" at pos x:%ld (%ld), y:%ld (%ld) - invert: %d", c, pos._x, pos._x / 8, pos._y, pos._y / 8, p.invert_);

  uint8_t x = pos._x / 8;
  uint8_t y = pos._y / 8;
  mode0_set_cursor(x, y);
  mode0_putc(c, p.invert_);
}

void picoTrackerGUIWindowImp::DrawString(const char *string, GUIPoint &pos,
                                         GUITextProperties &p, bool overlay) {
  Trace::Debug("draw string");
  mode0_set_cursor(pos._x, pos._y);
  mode0_print(string, p.invert_);
};

void picoTrackerGUIWindowImp::DrawRect(GUIRect &r) {
  Trace::Debug("GUI DrawRect call");
};

void picoTrackerGUIWindowImp::Clear(GUIColor &c, bool overlay) {
  mode0_color_t backgroundColor = GetColor(c);
  mode0_set_background(backgroundColor);
  mode0_clear(backgroundColor);
};

void picoTrackerGUIWindowImp::ClearRect(GUIRect &r) {
  Trace::Debug("GUI ClearRect call");
};

mode0_color_t picoTrackerGUIWindowImp::GetColor(GUIColor &c) {
  // Palette index should always be < 16. Wont check it.
  // TODO: should not be redefining the palette colors every call
  mode0_set_palette_color(c._paletteIndex, to_rgb565(c));
  return (mode0_color_t)c._paletteIndex;
}

void picoTrackerGUIWindowImp::SetColor(GUIColor &c) {
  mode0_set_foreground(GetColor(c));
};

void picoTrackerGUIWindowImp::Lock(){};

void picoTrackerGUIWindowImp::Unlock(){};

void picoTrackerGUIWindowImp::Flush() { mode0_draw_changed(); };

void picoTrackerGUIWindowImp::Invalidate() {
  picoTrackerEvent *event = new picoTrackerEvent();
  event->type_ = PICO_REDRAW;
  picoTrackerEventQueue::GetInstance()->Push(*event);
};

void picoTrackerGUIWindowImp::PushEvent(GUIEvent &event) {
  Trace::Debug("GUI PushEvent");
};

GUIRect picoTrackerGUIWindowImp::GetRect() {
  Trace::Debug("GUI GetRect");
  return GUIRect(0, 0, 320, 240);
}

void picoTrackerGUIWindowImp::ProcessEvent(picoTrackerEvent &event) {
  switch (event.type_) {
  case PICO_REDRAW:
    //        instance_->currentBuffer_^=0x01 ;
    instance_->_window->Update();
    //        gp_setFramebuffer(instance_->framebuffer_[instance_->currentBuffer_],1);
    break;
  }
}


void picoTrackerGUIWindowImp::ProcessButtonChange(uint16_t changeMask,
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
