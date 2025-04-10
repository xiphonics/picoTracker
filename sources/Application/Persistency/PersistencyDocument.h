#ifndef _PERSISTENCY_DOCUMENT_H_
#define _PERSISTENCY_DOCUMENT_H_

#include "Externals/yxml/yxml.h"
#include "System/FileSystem/FileSystem.h"
#include "System/FileSystem/PI_File.h"

class PersistencyDocument {
public:
  PersistencyDocument();
  bool Load(const char *filename);

  bool FirstChild();
  bool NextSibling();
  bool NextAttribute();
  bool HasContent();
  char *ElemName();

  char attrname_[64];
  char attrval_[64];
  char content_[129]; // 128 + \0
  yxml_ret_t r_;

  int version_;

private:
  inline static char stack_[1024];
  inline static yxml_t state_[1];
  PI_File *fp_;
};
#endif
