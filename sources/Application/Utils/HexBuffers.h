#ifndef _HEX_BUFFERS_H_
#define _HEX_BUFFERS_H_

#include "Application/Persistency/PersistencyDocument.h"
#include "Externals/TinyXML/tinyxml.h"

void saveHexBuffer(TiXmlNode *parent, const char *nodeName, unsigned char *src,
                   unsigned len);
void saveHexBuffer(TiXmlNode *parent, const char *nodeName, unsigned short *src,
                   unsigned len);
void saveHexBuffer(TiXmlNode *parent, const char *nodeName, unsigned int *src,
                   unsigned len);
void restoreHexBuffer(PersistencyDocument *doc, unsigned char *dst);

#endif
