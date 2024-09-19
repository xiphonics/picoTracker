#ifndef _HEX_BUFFERS_H_
#define _HEX_BUFFERS_H_

#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/TinyXML2/tinyxml2.h"
#include "Foundation/Types/Types.h"

void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   unsigned char *src, unsigned len);
void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   unsigned short *src, unsigned len);
void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   unsigned int *src, unsigned len);
void saveHexBuffer(tinyxml2::XMLPrinter *printer, const char *nodeName,
                   FourCC *src, unsigned len);
void restoreHexBuffer(PersistencyDocument *doc, unsigned char *dst);

#endif
