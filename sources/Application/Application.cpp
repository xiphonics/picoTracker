/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Application/Application.h"
#include "Application/AppWindow.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Application/Persistency/PersistenceConstants.h"
#include "Application/Persistency/PersistencyService.h"
#include "Services/Audio/Audio.h"
#include "Services/Midi/MidiService.h"
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"

#include <math.h>

Application *Application::instance_ = NULL;

Application::Application() {}

bool Application::Init(GUICreateWindowParams &params) {
  PersistencyService::GetInstance();

  ensurePTDirsExist();

  char projectName[MAX_PROJECT_NAME_LENGTH];
  initProject(projectName);

  window_ = AppWindow::Create(params, projectName);
  Audio *audio = Audio::GetInstance();
  audio->Init();

  return true;
};

// will put the current project name into the passed in char buffer projectName
// will return true if a new project was created (vs loading prev open project)
bool Application::initProject(char *projectName) {
  // read new proj name after reboot
  if (PersistencyService::GetInstance()->LoadCurrentProjectName(projectName) !=
      PERSIST_LOAD_FAILED) {
    if (PersistencyService::GetInstance()->Load(projectName) ==
        PERSIST_LOAD_FAILED) {
      Trace::Error("failed to load CURRENT proj: %s", projectName);
      if (strcmp(projectName, UNNAMED_PROJECT_NAME) == 0) {
        // untitled project is missing so need to create a new one
        if (PersistencyService::GetInstance()->CreateProject() !=
            PERSIST_SAVED) {
          Trace::Log("APPLICATION", "FAILED to create new UNTITLED project !!");
          // TODO: show user some sort of error message and how to recover from
          // this?
        }
      }
    }
    return false;
  } else {
    // need to create a new project and open it as no previously open
    // project state exists
    strcpy(projectName, UNNAMED_PROJECT_NAME); // default project name
    Trace::Log("APPLICATION", "create new project");
    // create  project
    if (PersistencyService::GetInstance()->CreateProject() != PERSIST_SAVED) {
      Trace::Log("APPLICATION", "FAILED to create new project !!");
      // TODO: show user some sort of error message and how to recover from
      // this?
    }
    return true;
  }
}

// ensure that all the directories required by picoTracker exist:
// /samples
// /projects
// /instruments
// /renders
void Application::ensurePTDirsExist() {
  auto fs = FileSystem::GetInstance();

  createIfNotExists(fs, PROJECTS_DIR);
  createIfNotExists(fs, SAMPLES_LIB_DIR);
  createIfNotExists(fs, INSTRUMENTS_DIR);
  createIfNotExists(fs, RENDERS_DIR);
  createIfNotExists(fs, THEMES_DIR);
}

void Application::createIfNotExists(FileSystem *fs, const char *path) {
  if (!fs->exists(path)) {
    fs->makeDir(path);
    Trace::Log("APPLICATION", "created %s std dir", path);
  }
}

GUIWindow *Application::GetWindow() { return window_; };

Application::~Application() { delete window_; }
