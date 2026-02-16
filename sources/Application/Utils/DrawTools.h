/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2026 xiphonics, inc.
 *
 * This file is part of the picoTracker firmware
 */

#include "Application/Views/BaseClasses/View.h"

void DrawBorder(View &view, int x, int y, int width, int height,
                const GUITextProperties &props, const char *charset);
void DrawDoubleBorder(View &view, int x, int y, int width, int height,
                      const GUITextProperties &props);
void DrawSingleBorder(View &view, int x, int y, int width, int height,
                      const GUITextProperties &props);
void DrawScrollBar(View &view, int topIndex, int totalItems, int pageSize);