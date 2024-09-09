#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "Foundation/T_Singleton.h"
#include "System/FileSystem/PicoFileSystem.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"

class Application : public T_Singleton<Application> {

public:
  Application();
  ~Application();
  bool Init(GUICreateWindowParams &params);

  GUIWindow *GetWindow();

protected:
  void initMidiInput();
  bool initProject(char *projectName);
  void ensurePTDirsExist();

private:
  GUIWindow *window_;
  static Application *instance_;
  void createIfNotExists(PicoFileSystem *picoFS, const char *path);
};

#endif
