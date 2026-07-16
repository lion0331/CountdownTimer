// Theme.h - GDI+ 主题绘制辅助
#pragma once
#include "Common.h"

void           Theme_InitFont(const wchar_t* face, int size);
void           Theme_CleanupFont();
Gdiplus::Font* Theme_GetFont();
HBRUSH         Theme_GetControlBrush();
