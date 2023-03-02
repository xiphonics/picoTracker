#ifndef HW_GUI_WINDOW_H_
#define HW_GUI_WINDOW_H_

#include "PICOEventQueue.h"
#include "UIFramework/Interfaces/I_GUIWindowImp.h"
#include "Adapters/PICO/ili9341/mode0.h"
#include <string>

class HWGUIWindowImp : public I_GUIWindowImp {

public:
  HWGUIWindowImp(GUICreateWindowParams &p);
  virtual ~HWGUIWindowImp();

public: // I_GUIWindowImp implementation
  virtual void SetColor(GUIColor &);
  virtual void DrawRect(GUIRect &);
  virtual void DrawChar(const char c, GUIPoint &pos, GUITextProperties &);
  virtual void DrawString(const char *string, GUIPoint &pos,
                          GUITextProperties &, bool overlay = false);
  virtual GUIRect GetRect();
  virtual void Invalidate();
  virtual void Flush();
  virtual void Lock();
  virtual void Unlock();
  virtual void Clear(GUIColor &, bool overlay = false);
  virtual void ClearRect(GUIRect &);
  virtual void PushEvent(GUIEvent &event);

  /*public: // Added functionality
    void ProcessQuit() ;
    void ProcessUserEvent(SDL_Event &event) ;
  */
  static void ProcessEvent(PICOEvent &event);
  static void ProcessButtonChange(uint16_t changeMask, uint16_t buttonMask);

protected:
  static mode0_color_t GetColor(GUIColor &c);

private:
};
#endif
