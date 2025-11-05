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
#include "System/FileSystem/FileSystem.h"

#include <math.h>

bool forceLoadUntitledProject = false;

ApplicationBase *ApplicationBase::instance_ = NULL;

bool ApplicationBase::Init(GUICreateWindowParams &params) {
  PersistencyService::GetInstance();

  ensurePTDirsExist();

  char projectName[MAX_PROJECT_NAME_LENGTH];
  initProject(projectName);

  window_ = AppWindow::Create(params, projectName);
  Audio *audio = Audio::GetInstance();
  audio->Init();

  // Initialize display brightness from config
  // Get the brightness value from config and apply it
  Config *config = Config::GetInstance();
  if (config) {
    Variable *v = config->FindVariable(FourCC::VarBacklightLevel);
    if (v) {
      unsigned char brightness = (unsigned char)v->GetInt();
      System::GetInstance()->SetDisplayBrightness(brightness);
      Trace::Log("PICOTRACKERSYSTEM", "Set display brightness to %d",
                 brightness);
    }
  }

  return true;
};

// will put the current project name into the passed in char buffer projectName
// will return true if a new project was created (vs loading prev open project)
bool ApplicationBase::initProject(char *projectName) {

  if (forceLoadUntitledProject) {
    Trace::Log("APPLICATION", "Force loading untitled project");
    FileSystem::GetInstance()->DeleteFile("/.current");
    forceLoadUntitledProject = false; // Reset the flag
  }

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
void ApplicationBase::ensurePTDirsExist() {
  auto fs = FileSystem::GetInstance();

  createIfNotExists(fs, PROJECTS_DIR);
  createIfNotExists(fs, SAMPLES_LIB_DIR);
  createIfNotExists(fs, INSTRUMENTS_DIR);
  createIfNotExists(fs, RENDERS_DIR);
  createIfNotExists(fs, THEMES_DIR);
  createIfNotExists(fs, RECORDINGS_DIR);
}

void ApplicationBase::createIfNotExists(FileSystem *fs, const char *path) {
  if (!fs->exists(path)) {
    fs->makeDir(path);
    Trace::Log("APPLICATION", "created %s std dir", path);
  }
}

GUIWindow *ApplicationBase::GetWindow() { return window_; };
