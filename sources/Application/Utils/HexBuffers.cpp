
#include "HexBuffers.h"
#include "Application/Utils/char.h"

#define XML_CUT_LENGTH 64

void prepareHexChunk(TiXmlElement &d, unsigned char *datasrc, int len) {

  bool singleValue = true;
  int singleValueData = -1;
  unsigned char hexBuffer[XML_CUT_LENGTH * 2 + 1];
  std::string buffer;

  char *hex = (char *)hexBuffer;
  for (int i = 0; i < len; i++) {
    hex2char(*datasrc, hex);
    if (singleValueData == -1) {
      singleValueData = *datasrc;
    } else {
      if (singleValueData != *datasrc) {
        singleValue = false;
      }
    };
    datasrc++;
    hex += 2;
  };
  buffer += (const char *)hexBuffer;
  if (singleValue) {
    d.SetAttribute("VALUE", singleValueData);
    d.SetAttribute("LENGTH", len);
  } else {
    TiXmlText text(buffer.c_str());
    d.InsertEndChild(text);
  }
}

void saveHexBuffer(TiXmlNode *parent, const char *nodeName, unsigned char *src,
                   unsigned len) {

  TiXmlElement data(nodeName);
  TiXmlNode *dataNode = parent->InsertEndChild(data);

  unsigned int count = len / XML_CUT_LENGTH;
  unsigned char *datasrc = (unsigned char *)src;

  for (unsigned i = 0; i < count; i++) {
    TiXmlElement d("DATA");
    prepareHexChunk(d, datasrc, XML_CUT_LENGTH);
    datasrc += XML_CUT_LENGTH;
    dataNode->InsertEndChild(d);
  };

  len -= count * XML_CUT_LENGTH;
  if (len > 0) {
    TiXmlElement d("DATA");
    prepareHexChunk(d, datasrc, len);
    dataNode->InsertEndChild(d);
  }
};

void saveHexBuffer(TiXmlNode *parent, const char *nodeName, unsigned int *src,
                   unsigned len) {
  saveHexBuffer(parent, nodeName, (unsigned char *)src, len * sizeof(int));
}

void saveHexBuffer(TiXmlNode *parent, const char *nodeName, unsigned short *src,
                   unsigned len) {
  saveHexBuffer(parent, nodeName, (unsigned char *)src, len * sizeof(short));
}

void restoreHexBuffer(PersistencyDocument *doc, unsigned char *destination) {
  unsigned char *dst = destination;

  bool child = doc->FirstChild();
  while (child) {
    bool hasAttr = doc->NextAttribute();
    if (hasAttr) {
      int data;
      int length = 0;
      while (hasAttr) {
        if (!strcmp(doc->attrname_, "VALUE")) {
          data = atoi(doc->attrval_);
        }
        if (!strcmp(doc->attrname_, "LENGTH")) {
          length = atoi(doc->attrval_);
        }
        hasAttr = doc->NextAttribute();
      }
      memset(dst, data, length);
      dst += length;
    } else {
      if (doc->HasContent()) {
        for (unsigned int i = 0; i < strlen(doc->content_) / 2; i++) {
          const char *src__ = doc->content_ + i * 2;
          unsigned char b1 = (c2h__(src__[0])) << 4;
          unsigned char b2 = c2h__(src__[1]);
          *dst++ = b1 + b2;
          //        char2hex(buffer+i*2,dst++) ;
        }
      }
    }
    child = doc->NextSibling();
  }
}
