#ifndef _RENDER_PROGRESS_MODAL_H_
#define _RENDER_PROGRESS_MODAL_H_

#include "Application/Player/Player.h"
#include "Application/Views/BaseClasses/ModalView.h"
#include <etl/string.h>

// Forward declarations
class GUIPoint;
class GUITextProperties;

// Progress message box with play time display
class RenderProgressModal : public ModalView {
public:
  // Constructor taking a view, title and message
  RenderProgressModal(View &view, const char *title, const char *message);

  // Virtual destructor
  virtual ~RenderProgressModal();

  // ModalView overrides
  virtual void DrawView();
  virtual void OnPlayerUpdate(PlayerEventType, unsigned int currentTick);
  virtual void OnFocus();
  virtual void ProcessButtonMask(unsigned short mask, bool pressed);
  virtual void AnimationUpdate();

private:
  // Title and message strings
  etl::string<20> title_;
  etl::string<32> message_;
};

#endif // _RENDER_PROGRESS_MODAL_H_