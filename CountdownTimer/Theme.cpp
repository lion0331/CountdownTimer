// Theme.cpp - GDI+ 字体管理实现
#include "Common.h"
#include "Engine.h"
#include "Theme.h"

using namespace Gdiplus;

static Font* g_pDisplayFont = nullptr;

void Theme_InitFont(const wchar_t* face, int size)
{
	delete g_pDisplayFont; g_pDisplayFont = nullptr;
	FontFamily family(face);
	if (family.GetLastStatus() != Ok)
	{
		FontFamily fallback(DEFAULT_FONT);
		g_pDisplayFont = new Font(&fallback, (REAL)size, FontStyleBold, UnitPoint);
	}
	else
	{
		g_pDisplayFont = new Font(&family, (REAL)size, FontStyleBold, UnitPoint);
	}
}

void Theme_CleanupFont()
{
	delete g_pDisplayFont; g_pDisplayFont = nullptr;
}

Font* Theme_GetFont()
{
	return g_pDisplayFont;
}

HBRUSH Theme_GetControlBrush()
{
	static HBRUSH hBr = CreateSolidBrush(CLR_BG_LIGHT);
	return hBr;
}
