
#include "HexBuffers.h"
#include "Application/Utils/char.h"

#define XML_CUT_LENGTH 64

void prepareHexChunk(tinyxml2::XMLPrinter *printer, unsigned char *datasrc,
                     int len) {

  bool singleValue = true;
  int singleValueData = -1;
  unsigned char hexBuffer[XML_CUT_LENGTH * 2 + 1] = "";
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
    printer->PushAttribute("VALUE", singleValueData);
    printer->PushAttribute("LENGTH", len);
  } else {
    printer->PushText(buffer.c_str());
  }
}

void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   unsigned char *src, unsigned len) {

  printer->OpenElement(nodeName);

  unsigned int count = len / XML_CUT_LENGTH;
  unsigned char *datasrc = (unsigned char *)src;

  for (unsigned i = 0; i < count; i++) {
    printer->OpenElement("DATA");
    prepareHexChunk(printer, datasrc, XML_CUT_LENGTH);
    datasrc += XML_CUT_LENGTH;
    printer->CloseElement();
  };

  len -= count * XML_CUT_LENGTH;
  if (len > 0) {
    printer->OpenElement("DATA");
    prepareHexChunk(printer, datasrc, len);
    printer->CloseElement();
  }
  printer->CloseElement();
};

void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   unsigned int *src, unsigned len) {
  saveHexBuffer(printer, nodeName, (unsigned char *)src, len * sizeof(int));
}

void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   unsigned short *src, unsigned len) {
  saveHexBuffer(printer, nodeName, (unsigned char *)src, len * sizeof(short));
}

void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   FourCC *src, unsigned len) {
  saveHexBuffer(printer, nodeName, (unsigned char *)src, len * sizeof(short));
}

void restoreHexBuffer(PersistencyDocument *doc, unsigned char *destination) {
  unsigned char *dst = destination;

  bool child = doc->FirstChild();
  while (child) {
    bool hasAttr = doc->NextAttribute();
    if (hasAttr) {
      int data = 0;
      int length = 0;
      bool gotData = false;
      while (hasAttr) {
        if (!strcmp(doc->attrname_, "VALUE")) {
          data = atoi(doc->attrval_);
          gotData = true;
        }
        if (!strcmp(doc->attrname_, "LENGTH")) {
          length = atoi(doc->attrval_);
        }
        hasAttr = doc->NextAttribute();
      }
      if (gotData) {
        memset(dst, data, length);
      }
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
