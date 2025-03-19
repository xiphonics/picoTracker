#ifndef _RENDER_PROGRESS_MODAL_H_
#define _RENDER_PROGRESS_MODAL_H_

#include "Application/Player/Player.h"
#include "Application/Views/BaseClasses/ModalView.h"
#include <etl/string.h>

// Forward declarations
class GUIPoint;
class GUITextProperties;

// Progress message box with render progress display
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
  // Helper method to draw the render progress
  void drawRenderProgress(GUIPoint &pos, GUITextProperties &props);

  // Helper method to check if song has looped back to start
  bool hasSongLooped(unsigned int currentTick);

  // Helper method to calculate samples per buffer based on tempo
  float calculateSamplesPerBuffer(int tempo);

  // Title and message strings
  etl::string<20> title_;
  etl::string<32> message_;

  // Track the last tick we saw to detect loops
  unsigned int lastTick_;

  // Track total rendered samples (calculated from player updates)
  float totalSamples_;

  // Flag to indicate if we've detected a loop
  bool loopDetected_;

  int tempo_ = 0;

  // Constants for sample rate calculations
  static const int SAMPLE_RATE = 44100;
  static const int AUDIO_SLICES_PER_STEP = 6;
};

#endif // _RENDER_PROGRESS_MODAL_H_