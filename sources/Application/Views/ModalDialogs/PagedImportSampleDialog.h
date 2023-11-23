#ifndef _IMPORT_SAMPLE_DIALOG_H_
#define _IMPORT_SAMPLE_DIALOG_H_

#include "Application/Views/BaseClasses/ModalView.h"
#include "Foundation/T_SimpleList.h"
#include "System/FileSystem/FileSystem.h"
#include <string>
#include <vector>

const static char *sampleLib_ = "/samplelib";


struct FileListItem {
public:
	const char* name;
	bool IsDirectory;
};


class PagedImportSampleDialog : public ModalView {
public:
	PagedImportSampleDialog(View &view) ;
	virtual ~PagedImportSampleDialog() ;

	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType ,unsigned int currentTick) ;
	virtual void OnFocus() ;
	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;

protected:
	void setCurrentFolder(Path *path) ;
	void warpToNextSample(int dir) ;
	void import(Path &element) ;
	void preview(Path &element) ;
private:
	const std::vector<FileListItem> fileList_ {};
	int currentSample_ ;
	int topIndex_ ;
	int toInstr_ ;
	int selected_ ;
	Path currentPath_ { "/samplelib"};

} ;

#endif

