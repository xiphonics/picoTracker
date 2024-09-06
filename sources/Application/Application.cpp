#include "Application/Application.h"
#include "Application/AppWindow.h"
#include "Application/Commands/CommandDispatcher.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistencyService.h"
#include "Services/Audio/Audio.h"
#include "Services/Midi/MidiService.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"

#include <math.h>

Application *Application::instance_ = NULL;

Application::Application() {}

void Application::initMidiInput() {
  const char *preferedDevice =
      Config::GetInstance()->GetValue("MIDICTRLDEVICE");

  auto midiInService = MidiService::GetInstance();
  for (midiInService->InBegin(); !midiInService->InIsDone();
       midiInService->InNext()) {
    MidiInDevice &in = midiInService->InCurrentItem();
    if ((preferedDevice) &&
        (!strncmp(in.GetName(), preferedDevice, strlen(preferedDevice)))) {
      if (in.Init()) {
        if (in.Start()) {
          Trace::Log("MIDI", "Controlling activated for MIDI interface %s",
                     in.GetName());
        } else {
          in.Close();
        }
      }
    }
  }
}

bool Application::Init(GUICreateWindowParams &params) {
  PersistencyService::GetInstance();

  ensurePTDirsExist();

  char projectName[MAX_PROJECT_NAME_LENGTH];
  initProject(projectName);

  window_ = AppWindow::Create(params, projectName);
  Audio *audio = Audio::GetInstance();
  audio->Init();
  CommandDispatcher::GetInstance()->Init();
  initMidiInput();
  return true;
};

// returns the current project name in the passed in char buffer
void Application::initProject(char *projectName) {
  // read new proj name after reboot
  if (PersistencyService::GetInstance()->LoadCurrentProjectName(projectName) !=
      PERSIST_LOAD_FAILED) {
    if (PersistencyService::GetInstance()->Load(projectName) ==
        PERSIST_LOAD_FAILED) {
      Trace::Error("failed to load CURRENT proj: %s\n", projectName);
    }
  } else { // need to create a new project and open it as no previously open
           // project state exists
    strcpy(projectName, "new_project");
    Trace::Log("APPLICATION", "create new project\n");
    // create  project
    auto res = PersistencyService::GetInstance()->Save(projectName, true);
    if (res != PERSIST_PROJECT_EXISTS) {
      Trace::Log("APPLICATION", "created new proj: %s\n", projectName);
    } else {
      Trace::Log("APPLICATION",
                 "failed to create new proj already exists: %s\n", projectName);
    }
  }
}

// ensure that all the directories required by picoTracker exist:
// /samples
// /projects
// /instruments
// /renders
void Application::ensurePTDirsExist() {
  auto picoFS = PicoFileSystem::GetInstance();

  createIfNotExists(picoFS, PROJECTS_DIR);
  createIfNotExists(picoFS, SAMPLES_LIB_DIR);
  createIfNotExists(picoFS, INSTRUMENTS_DIR);
  createIfNotExists(picoFS, RENDERS_DIR);
}

void Application::createIfNotExists(PicoFileSystem *picoFS, const char *path) {
  if (!picoFS->exists(path)) {
    picoFS->makeDir(path);
    Trace::Log("APPLICATION", "created %s std dir\n", path);
  }
}

GUIWindow *Application::GetWindow() { return window_; };

Application::~Application() { delete window_; }
