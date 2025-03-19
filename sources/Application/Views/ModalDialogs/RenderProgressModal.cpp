#include "RenderProgressModal.h"
#include "Application/Player/Player.h"
#include "Application/Views/BaseClasses/View.h"
#include "MessageBox.h"
#include "UIFramework/BasicDatas/GUIPoint.h"

RenderProgressModal::RenderProgressModal(View &view, const char *title,
                                         const char *message)
    : ModalView(view), title_(title), message_(message) {}

RenderProgressModal::~RenderProgressModal() {}

void RenderProgressModal::DrawView() {
  // Calculate window size
  int width = title_.size() > message_.size() ? title_.size() : message_.size();
  width = width > 16 ? width : 16; // Minimum width for time display
  SetWindow(width, 4); // Height of 4 for title, message, time, and button

  // Draw title
  int y = 0;
  int x = (width - title_.size()) / 2;
  GUITextProperties props;
  SetColor(CD_ERROR);
  DrawString(x, y, title_.c_str(), props);

  // Draw message
  y++;
  x = (width - message_.size()) / 2;
  DrawString(x, y, message_.c_str(), props);

  // Draw play time
  y++;
  Player *player = Player::GetInstance();
  GUIPoint timePos(width / 2 - 2, y); // Center the time display
  drawPlayTime(player, timePos, props);

  // Draw OK button
  SetColor(CD_NORMAL);
  y++;
  props.invert_ = true;
  x = width / 2 - 1; // Center the OK button
  DrawString(x, y, "OK", props);
}

void RenderProgressModal::OnPlayerUpdate(PlayerEventType,
                                         unsigned int currentTick) {
  // redraw just the elapsed time display
  int width = title_.size() > message_.size() ? title_.size() : message_.size();
  width = width > 16 ? width : 16; // Minimum width for time display
  int y = 2;
  Player *player = Player::GetInstance();
  GUITextProperties props;
  SetColor(CD_ERROR);
  GUIPoint timePos(width / 2 - 2, y); // Center the time display
  drawPlayTime(player, timePos, props);
}

void RenderProgressModal::OnFocus() {}

void RenderProgressModal::ProcessButtonMask(unsigned short mask, bool pressed) {
  if (mask & EPBM_ENTER && pressed) {
    EndModal(MBL_OK);
  }
  isDirty_ = true;
}

void RenderProgressModal::AnimationUpdate() {
  // Mark as dirty to update the time display
  isDirty_ = true;
}