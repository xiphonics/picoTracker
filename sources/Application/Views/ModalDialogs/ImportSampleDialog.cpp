#include "ImportSampleDialog.h"
#include "Application/Instruments/SampleInstrument.h"
#include "Application/Instruments/SamplePool.h"
#include "pico/multicore.h"
#include <memory>

#define LIST_SIZE 15
#define LIST_WIDTH 24

bool ImportSampleDialog::initStatic_ = false;
Path ImportSampleDialog::sampleLib_("");
Path ImportSampleDialog::currentPath_("");

static const char *buttonText[3] = {"Listen", "Import", "Exit"};

ImportSampleDialog::ImportSampleDialog(View &view) : ModalView(view) {
  if (!initStatic_) {
    const char *slpath = SamplePool::GetInstance()->GetSampleLib();
    sampleLib_ = Path(slpath);
    currentPath_ = Path(slpath);
    initStatic_ = true;
  }
  selected_ = 0;

  sampleList_.SetOwnership(true);
}

ImportSampleDialog::~ImportSampleDialog() { sampleList_.Empty(); }

void ImportSampleDialog::DrawView() {
  SetWindow(LIST_WIDTH, LIST_SIZE + 3);

  GUITextProperties props;

  // Draw title

#ifdef SHOW_MEM_USAGE
  char title[40];

  SetColor(CD_NORMAL);

  sprintf(title, "MEM [%d]", System::GetInstance()->GetMemoryUsage());
  GUIPoint pos = GUIPoint(0, 0);
  w_.DrawString(title, pos, props);

#endif

  // Draw samples

  int x = 1;
  int y = 1;

  if (currentSample_ < topIndex_) {
    topIndex_ = currentSample_;
  };
  if (currentSample_ >= topIndex_ + LIST_SIZE) {
    topIndex_ = currentSample_;
  };

  IteratorPtr<Path> it(sampleList_.GetIterator());
  int count = 0;
  char buffer[64];
  for (it->Begin(); !it->IsDone(); it->Next()) {
    if ((count >= topIndex_) && (count < topIndex_ + LIST_SIZE)) {
      Path &current = it->CurrentItem();
      const std::string p = current.GetName();

      if (count == currentSample_) {
        SetColor(CD_HILITE2);
        props.invert_ = true;
      } else {
        SetColor(CD_NORMAL);
        props.invert_ = false;
      }
      if (!current.IsDirectory()) {
        strcpy(buffer, p.c_str());
      } else {
        buffer[0] = '[';
        strcpy(buffer + 1, p.c_str());
        strcat(buffer, "]");
      }
      buffer[LIST_WIDTH - 1] = 0;
      DrawString(x, y, buffer, props);
      y += 1;
    }
    count++;
  };

  y = LIST_SIZE + 2;
  int offset = 7;

  SetColor(CD_NORMAL);

  for (int i = 0; i < 3; i++) {
    const char *text = buttonText[i];
    x = (offset * (i + 1) - strlen(text) / 2) - 2;
    props.invert_ = (i == selected_) ? true : false;
    DrawString(x, y, text, props);
  }
};

void ImportSampleDialog::warpToNextSample(int direction) {

  currentSample_ += direction;
  int size = sampleList_.Size();
  if (currentSample_ < 0)
    currentSample_ += size;
  if (currentSample_ >= size)
    currentSample_ -= size;
  isDirty_ = true;
}

void ImportSampleDialog::OnPlayerUpdate(PlayerEventType,
                                        unsigned int currentTick){};

void ImportSampleDialog::OnFocus() {
  Path current(currentPath_);
  setCurrentFolder(&current);
  toInstr_ = viewData_->currentInstrument_;
};

void ImportSampleDialog::preview(Path &element) {
  Player::GetInstance()->StartStreaming(element);
}

void ImportSampleDialog::import(Path &element) {

  SamplePool *pool = SamplePool::GetInstance();

#ifdef PICO_BUILD
  // Pause core1 in order to be able to write to flash and ensure core1 is
  // not reading from it, it also disables IRQs on it
  // https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#multicore_lockout
  multicore_lockout_start_blocking();
#endif
  int sampleID = pool->ImportSample(element);
#ifdef PICO_BUILD
  multicore_lockout_end_blocking();
#endif

  if (sampleID >= 0) {
    I_Instrument *instr =
        viewData_->project_->GetInstrumentBank()->GetInstrument(toInstr_);
    if (instr->GetType() == IT_SAMPLE) {
      SampleInstrument *sinstr = (SampleInstrument *)instr;
      sinstr->AssignSample(sampleID);
      toInstr_ = viewData_->project_->GetInstrumentBank()->GetNext();
    };
  } else {
    Trace::Error("failed to import sample");
  };
  isDirty_ = true;
};

void ImportSampleDialog::ProcessButtonMask(unsigned short mask, bool pressed) {

  if (!pressed)
    return;

  if (mask & EPBM_B) {
    if (mask & EPBM_UP)
      warpToNextSample(-LIST_SIZE);
    if (mask & EPBM_DOWN)
      warpToNextSample(LIST_SIZE);
  } else {

    // A modifier
    if (mask & EPBM_A) {
      IteratorPtr<Path> it(sampleList_.GetIterator());
      int count = 0;
      Path *element = 0;
      for (it->Begin(); !it->IsDone(); it->Next()) {
        if (count++ == currentSample_) {
          element = &it->CurrentItem();
        }
      }

      if ((selected_ != 2) && (element->IsDirectory())) {
        if (element->GetName() == "..") {
          if (currentPath_.GetPath() == sampleLib_.GetPath()) {
            //				EndModal(true) ;
          } else {
            Path parent = element->GetParent().GetParent();
            setCurrentFolder(&parent);
          }
        } else {
          Path child(element->GetPath());
          setCurrentFolder(&child);
        }
        isDirty_ = true;
        return;
      }

      switch (selected_) {
      case 0: // preview
        preview(*element);
        break;
      case 1: // import
        import(*element);
        break;
      case 2: // Exit ;
        EndModal(0);
        break;
      }
    } else {
      // R Modifier
      if (mask & EPBM_R) {
      } else {
        // No modifier
        if (mask == EPBM_UP)
          warpToNextSample(-1);
        if (mask == EPBM_DOWN)
          warpToNextSample(1);
        if (mask == EPBM_LEFT) {
          selected_ -= 1;
          if (selected_ < 0)
            selected_ += 3;
          isDirty_ = true;
        }
        if (mask == EPBM_RIGHT) {
          selected_ = (selected_ + 1) % 3;
          isDirty_ = true;
        }
      }
    }
  }
}

void ImportSampleDialog::setCurrentFolder(Path *path) {

  Path formerPath(currentPath_);

  topIndex_ = 0;
  currentSample_ = 0;

  currentPath_ = Path(*path);
  sampleList_.Empty();
  if (path) {
    int count = 0;
    std::unique_ptr<I_Dir> dir(
        FileSystem::GetInstance()->Open(path->GetPath().c_str()));
    if (dir) {
      dir->GetContent("*");
      dir->Sort();
      IteratorPtr<Path> it(dir->GetIterator());
      for (it->Begin(); !it->IsDone(); it->Next()) {
        Path &current = it->CurrentItem();
        if (current.IsDirectory()) {
          if (current.GetName().substr(0, 1) != "." ||
              current.GetName() == "..") {
            ;
            Path *sample = new Path(current);
            sampleList_.Insert(sample);
            if (!formerPath.Compare(current)) {
              currentSample_ = count;
            }
            count++;
          }
        }
      }
      for (it->Begin(); !it->IsDone(); it->Next()) {
        Path &current = it->CurrentItem();
        if (!current.IsDirectory()) {
          if (current.Matches("*.wav") && current.GetName()[0] != '.') {
            Path *sample = new Path(current);
            sampleList_.Insert(sample);
          }
        };
      }
    }
  }
}
