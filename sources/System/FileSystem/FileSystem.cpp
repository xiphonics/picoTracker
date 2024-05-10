#include "FileSystem.h"
#include "Application/Utils/wildcard.h"
#include "System/Console/Trace.h"
#include <algorithm>

using namespace std;

Path::Path() : type_(FT_UNKNOWN), gotType_(false) {
  path_ = (char *)malloc(1);
  strcpy(path_, "");
};

Path::Path(const char *path) : type_(FT_UNKNOWN), gotType_(false) {
  path_ = (char *)malloc((int)strlen(path) + 1);
  strcpy(path_, path);
};

Path::Path(const std::string &path) : type_(FT_UNKNOWN), gotType_(false) {
  path_ = (char *)malloc(path.size() + 1);
  strcpy(path_, path.c_str());
};

Path::Path(const Path &other) {
  path_ = (char *)SYS_MALLOC((int)strlen(other.path_) + 1);
  strcpy(path_, other.path_);
  gotType_ = other.gotType_;
  type_ = other.type_;
};

Path &Path::operator=(const Path &other) {
  SAFE_FREE(path_);
  path_ = (char *)SYS_MALLOC((int)strlen(other.path_) + 1);
  strcpy(path_, other.path_);
  return *this;
};

Path::~Path() { SYS_FREE(path_); };

std::string Path::GetPath() const {
  if (path_) {
    return std::string(path_);
  }
  return "";
};

std::string Path::GetCanonicalPath() {
  std::string copy = GetPath();
  std::string::size_type pos;
  while ((pos = copy.find("\\")) != std::string::npos) {
    std::string rpart = copy.substr(pos + 1);
    copy = copy.substr(0, pos);
    copy += "/";
    copy += rpart;
  };
  return copy;
};

Path Path::Descend(const std::string &leaf) {
  std::string currentPath = GetPath();
  if (currentPath[currentPath.size() - 1] != '/') {
    currentPath += "/";
  }
  return Path(currentPath + leaf);
}

void Path::getType() {
  if (!gotType_) {
    gotType_ = true;
    type_ = FileSystem::GetInstance()->GetFileType(path_);
  };
};

std::string Path::GetName() {

  unsigned int index = 0;
  for (unsigned int i = 0; i < strlen(path_); i++) {
    if (path_[i] == '/') {
      index = i + 1;
    };
  };
  //	if (index!=0) index++ ;
  return std::string(path_ + index);
};

int Path::Compare(const Path &other) { return strcasecmp(path_, other.path_); };

bool Path::Exists() {
  getType();
  return type_ != FT_UNKNOWN;
};

bool Path::IsFile() {
  getType();
  return type_ == FT_FILE;
};

bool Path::IsDirectory() {
  getType();
  return type_ == FT_DIR;
};

bool Path::Matches(const char *pattern) {
  std::string name = GetName();
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return wildcardfit(pattern, name.c_str()) == 1;
};

Path Path::GetParent() {
  std::string current = GetCanonicalPath();
  std::string::size_type index = current.rfind("/", current.size() - 2);
  std::string parentPath = current.substr(0, index);
  Path parent(parentPath);
  return parent;
}

I_PagedDir::I_PagedDir() {};
I_PagedDir::~I_PagedDir() {};

I_PagedDir::I_PagedDir(const char *path) {};
