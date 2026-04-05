#pragma once

#include "Application/AppWindow.h"
#include "Application/Application.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Externals/etl/include/etl/vector.h"
#include "System/Console/Trace.h"
#include "System/FileSystem/FileSystem.h"

#define PROTECT_SCRATCH 1

#ifdef DEBUG
#define PROTECT_SCRATCH 1
#endif

constexpr size_t MEMORYPOOL_SCRATCH_SIZE = 1024;

class MemoryPool {
private:
  static char buffer_[MEMORYPOOL_SCRATCH_SIZE];
#if PROTECT_SCRATCH
  static bool used_;
#endif

public:
  static etl::vector<int, MAX_FILE_INDEX_SIZE> fileIndexList;

  // shared fileIndexList for file operations to avoid needing to allocate on
  // the stack in multiple places or have multiple instances of
  static void *acquire() {
#if PROTECT_SCRATCH
    if (used_) {
      Trace::Error("MemoryPool is already in use!");
      Application *app = Application::GetInstance();
      AppWindow *window = (AppWindow *)app->GetWindow();
      MessageBox *mb =
          MessageBox::Create(*(window->getCurrentView()),
                             "MemoryPool is already in use!", MBBF_OK);
      window->getCurrentView()->DoModal(mb);
    }
    used_ = true;
#endif
    return buffer_;
  }

  static void release() {
#if PROTECT_SCRATCH
    used_ = false;
#endif
  }
};