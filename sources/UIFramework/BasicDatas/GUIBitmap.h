/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2018 Discodirt
 * Copyright (c) 2024 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#ifndef _GUIBITMAP_H_
#define _GUIBITMAP_H_

#include "UIFramework/BasicDatas/GUIResourceID.h"
#include "UIFramework/Interfaces/I_GUIBitmapImp.h"

// Class for Bitmap description in GUI. The bitmap is drawable so
// it implements I_GUIGraphics

class GUIBitmap : public I_GUIGraphics {

protected:
  // We protect the constructors coz zzeeee
  // Class should be instanciated throught static call

  GUIBitmap(I_GUIBitmapImp &);

public:
  // Destructor

  virtual ~GUIBitmap();

  // I_GUIGraphics implementation

  virtual void DrawLine(long, long, long, long);
  virtual void DrawLine(GUIPoint &pt1, GUIPoint &pt2);
  virtual void SetColor(GUIColor &);
  virtual void DrawRect(GUIRect &);
  virtual void DrawBitmap(GUIBitmap &, GUIPoint &p);
  virtual void StretchBitmap(GUIBitmap &, GUIRect &srcR, GUIRect &dstR);
  virtual void SelectFont(int type, int size);
  virtual void DrawString(char *string, const GUIPoint &pos,
                          const GUITextProperties &props);
  virtual int GetStringWidth(char *string);

  virtual GUIRect GetRect();
  virtual void SetRect(GUIRect &);
  virtual void SetClipRect(GUIRect &r);
  virtual void Invalidate();
  virtual void Flush();

  // Additional calls

  void StretchENGBitmap(ENGBitmap &, GUIRect &srcRect, GUIRect &dstRect);

  // Returns the implementation used to actually do the job. Don't
  // use this as the result you get might be system dependent

  I_GUIBitmapImp *GetImp() { return _imp; };

public:
  // Static call to create Bitmaps

  static GUIBitmap *CreateFromResource(GUIResourceID &);
  static GUIBitmap *CreateBitmap(int width, int height);

private:
  I_GUIBitmapImp *_imp; // The imp we rely on to provide the bitmap services
};

#endif
