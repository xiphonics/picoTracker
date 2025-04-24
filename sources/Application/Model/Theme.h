#ifndef _THEME_H_
#define _THEME_H_

#include "Application/Model/Config.h"
#include "Application/Persistency/Persistent.h"
#include "Externals/etl/include/etl/string.h"
#include "Foundation/Types/Types.h"
#include "Foundation/Variables/Variable.h"

// Use constants from PersistenceConstants.h

class Theme : public Persistent {
public:
    Theme();
    ~Theme();

    // Save current theme settings from Config
    void SaveFromConfig();
    
    // Apply theme settings to Config
    void ApplyToConfig();

    // Set theme name
    void SetName(const char* name);
    
    // Get theme name
    const char* GetName() const;

protected:
    // Persistent interface implementation
    virtual void SaveContent(tinyxml2::XMLPrinter *printer);
    virtual void RestoreContent(PersistencyDocument *doc);

private:
    // Theme name
    etl::string<32> name_;
    
    // Store theme settings
    int fontValue_;
    int bgColor_;
    int fgColor_;
    int hi1Color_;
    int hi2Color_;
    int consoleColor_;
    int cursorColor_;
    int infoColor_;
    int warnColor_;
    int errorColor_;
    int playColor_;
    int muteColor_;
    int songViewFEColor_;
    int songView00Color_;
    int rowColor_;
    int row2Color_;
    int majorBeatColor_;
};

#endif // _THEME_H_
