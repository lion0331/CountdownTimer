// main.cpp
#include "Common.h"
#include "Engine.h"
#include "Theme.h"
#include "TrayIcon.h"

using namespace Gdiplus;

static void UpdateLayered(HWND hwnd);

struct MainWindow
{
	HWND hwnd = nullptr;
	CountdownEngine engine;
	AppSettings     settings;
	TrayIcon        tray;
	HICON hIcon = nullptr;
	int inputMode = 0;
	int editMode = 0;
	int editH = 0, editM = 5, editS = 0;
	SYSTEMTIME editDateST = {};
	int editField = 0;
	wchar_t editPrompt[128] = L"";
	bool dragging = false;
	int dragX = 0, dragY = 0;
};
static MainWindow g;

static void SetAutoStart(bool enable)
{
	HKEY hKey;
	const wchar_t* runKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	if (enable)
	{
		if (RegCreateKeyExW(HKEY_CURRENT_USER, runKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS)
		{
			wchar_t exePath[MAX_PATH];
			GetModuleFileNameW(nullptr, exePath, MAX_PATH);
			RegSetValueExW(hKey, L"CountdownTimer", 0, REG_SZ, (LPBYTE)exePath, (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t)));
			RegCloseKey(hKey);
		}
	}
	else
	{
		if (RegOpenKeyExW(HKEY_CURRENT_USER, runKey, 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
		{
			RegDeleteValueW(hKey, L"CountdownTimer");
			RegCloseKey(hKey);
		}
	}
}

static void AutoRestoreCountdown()
{
	if (!g.settings.wasRunning) return;
	g.inputMode = g.settings.inputMode;
	if (g.inputMode == 0)
	{
		if (g.settings.savedRemainingMs == 0) return;
		g.engine.RestoreFromRemainingMs(g.settings.savedRemainingMs);
	}
	else
	{
		if (g.settings.lastDateYear == 0) return;
		g.editDateST.wYear = g.settings.lastDateYear; g.editDateST.wMonth = g.settings.lastDateMonth;
		g.editDateST.wDay = g.settings.lastDateDay; g.editDateST.wHour = g.settings.lastTimeHour;
		g.editDateST.wMinute = g.settings.lastTimeMin; g.editDateST.wSecond = g.settings.lastTimeSec;
		g.engine.SetTargetDate(g.editDateST);
	}
	g.engine.Start(); g.editMode = 0;
	g.tray.UpdateTip(g.settings.promptText);
	UpdateLayered(g.hwnd);
}

static void StartCountdown()
{
	g.editMode = 0;
	g.settings.lastHours = g.editH; g.settings.lastMinutes = g.editM; g.settings.lastSeconds = g.editS;
	g.settings.inputMode = g.inputMode;
	if (g.inputMode == 0)
	{
		if (g.editH == 0 && g.editM == 0 && g.editS == 0)return; g.engine.SetDuration(g.editH, g.editM, g.editS);
	}
	else
	{
		g.settings.lastDateYear = g.editDateST.wYear; g.settings.lastDateMonth = g.editDateST.wMonth;
		g.settings.lastDateDay = g.editDateST.wDay; g.settings.lastTimeHour = g.editDateST.wHour;
		g.settings.lastTimeMin = g.editDateST.wMinute; g.settings.lastTimeSec = g.editDateST.wSecond;
		FILETIME ftT = {}, ftN = {}; SystemTimeToFileTime(&g.editDateST, &ftT); GetSystemTimeAsFileTime(&ftN);
		if (*(ULONGLONG*)&ftT <= *(ULONGLONG*)&ftN)
		{
			UpdateLayered(g.hwnd); return;
		}
		g.engine.SetTargetDate(g.editDateST);
	}
	g.engine.Start(); g.settings.Save(); g.tray.UpdateTip(g.settings.promptText); UpdateLayered(g.hwnd);
}

static void PauseResume()
{
	if (g.engine.IsRunning())g.engine.Pause(); else StartCountdown(); UpdateLayered(g.hwnd);
}
static void DoReset()
{
	g.engine.Reset(); g.editMode = 0; g.tray.UpdateTip(g.settings.promptText); UpdateLayered(g.hwnd);
}
static void OnFinished()
{
	if (g.settings.useCustomSound && wcslen(g.settings.soundPath) > 0)PlaySoundW(g.settings.soundPath, nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
	else for (int i = 0; i < 3; i++)
	{
		MessageBeep(MB_ICONEXCLAMATION); Sleep(300);
	}
	g.tray.ShowBalloon(L"\u5012\u8ba1\u65f6", L"\u5012\u8ba1\u65f6\u5df2\u7ed3\u675f\uff01");
	g.tray.UpdateTip(L"\u5012\u8ba1\u65f6 - \u5df2\u5b8c\u6210\uff01");
	UpdateLayered(g.hwnd);
}

static void BeginEditDuration()
{
	if (g.engine.IsRunning())return; g.editMode = 1; g.editField = 0; g.editH = g.settings.lastHours; g.editM = g.settings.lastMinutes; g.editS = g.settings.lastSeconds; if (g.editH == 0 && g.editM == 0 && g.editS == 0)g.editM = 5; UpdateLayered(g.hwnd);
}
static void BeginEditDate()
{
	if (g.engine.IsRunning())return; g.editMode = 2; g.editField = 0; if (g.settings.lastDateYear > 0)
	{
		g.editDateST.wYear = g.settings.lastDateYear; g.editDateST.wMonth = g.settings.lastDateMonth; g.editDateST.wDay = g.settings.lastDateDay; g.editDateST.wHour = g.settings.lastTimeHour; g.editDateST.wMinute = g.settings.lastTimeMin; g.editDateST.wSecond = g.settings.lastTimeSec;
	}
	else
	{
		GetLocalTime(&g.editDateST); g.editDateST.wMinute += 5; if (g.editDateST.wMinute >= 60)
		{
			g.editDateST.wMinute -= 60; g.editDateST.wHour++;
		}g.editDateST.wSecond = 0;
	}UpdateLayered(g.hwnd);
}
static void BeginEditPrompt()
{
	if (g.engine.IsRunning())return; g.editMode = 3; wcscpy_s(g.editPrompt, g.settings.promptText); UpdateLayered(g.hwnd);
}
static void CommitEdit()
{
	if (g.editMode == 3)
	{
		wcscpy_s(g.settings.promptText, g.editPrompt); g.editMode = 0; g.tray.UpdateTip(g.settings.promptText); g.settings.Save(); UpdateLayered(g.hwnd); return;
	}g.settings.Save(); StartCountdown();
}
static void CancelEdit()
{
	g.editMode = 0; UpdateLayered(g.hwnd);
}
static void AdjHour(int d)
{
	g.editH += d; if (g.editH < 0)g.editH = 0; if (g.editH > 99)g.editH = 99; UpdateLayered(g.hwnd);
}
static void AdjMin(int d)
{
	int t = g.editH * 60 + g.editM + d; if (t < 0)t = 0; g.editH = t / 60; g.editM = t % 60; UpdateLayered(g.hwnd);
}
static void AdjSec(int d)
{
	int t = g.editH * 3600 + g.editM * 60 + g.editS + d; if (t < 0)t = 0; g.editH = t / 3600; g.editM = (t / 60) % 60; g.editS = t % 60; UpdateLayered(g.hwnd);
}
static void AdjDate(int dH)
{
	FILETIME ft; SystemTimeToFileTime(&g.editDateST, &ft); *(ULONGLONG*)&ft += (LONGLONG)dH * 36000000000ULL; FileTimeToSystemTime(&ft, &g.editDateST); UpdateLayered(g.hwnd);
}

static HMENU BuildContextMenu()
{
	HMENU m = CreatePopupMenu();
	AppendMenuW(m, MF_STRING, 1001, g.inputMode == 0 ? L"\u8bbe\u7f6e\u65f6\u957f..." : L"\u8bbe\u7f6e\u65e5\u671f\u65f6\u95f4...");
	AppendMenuW(m, MF_STRING, 1005, L"\u7f16\u8f91\u63d0\u793a\u6587\u672c...");
	AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(m, MF_STRING, 1003, g.engine.IsRunning() ? L"\u6682\u505c" : L"\u5f00\u59cb");
	AppendMenuW(m, MF_STRING, 1004, L"\u91cd\u7f6e");
	AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(m, MF_STRING, 3001, L"\u65f6\u95f4\u989c\u8272...");
	AppendMenuW(m, MF_STRING, 3002, L"\u65f6\u95f4\u5b57\u4f53...");
	AppendMenuW(m, MF_STRING, 3003, L"\u63d0\u793a\u989c\u8272...");
	AppendMenuW(m, MF_STRING, 3004, L"\u63d0\u793a\u5b57\u4f53...");
	AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(m, MF_STRING | (g.settings.alwaysOnTop ? MF_CHECKED : 0), 2000, L"\u7f6e\u9876");
	AppendMenuW(m, MF_STRING | (g.settings.minimizeToTray ? MF_CHECKED : 0), 2001, L"\u5173\u95ed\u5230\u6258\u76d8");
	AppendMenuW(m, MF_STRING | (g.settings.autoStart ? MF_CHECKED : 0), 2003, L"\u5f00\u673a\u542f\u52a8");
	AppendMenuW(m, MF_STRING | (g.settings.centerOnScreen ? MF_CHECKED : 0), 2004, L"\u5c45\u4e2d");
	AppendMenuW(m, MF_SEPARATOR, 0, nullptr);
	AppendMenuW(m, MF_STRING, 2002, L"\u9000\u51fa");
	return m;
}

static void HandleMenuCmd(WORD id)
{
	switch (id)
	{
	case 1001:if (g.inputMode == 0)BeginEditDuration(); else BeginEditDate(); break;
	case 1003:PauseResume(); break;
	case 1004:DoReset(); break;
	case 1005:BeginEditPrompt(); break;
	case 2000:g.settings.alwaysOnTop = !g.settings.alwaysOnTop; g.settings.Save(); SetWindowPos(g.hwnd, g.settings.alwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW); break;
	case 2001:g.settings.minimizeToTray = !g.settings.minimizeToTray; g.settings.Save(); break;
	case 2003:g.settings.autoStart = !g.settings.autoStart; g.settings.Save(); SetAutoStart(g.settings.autoStart); break;
	case 2004:
		g.settings.centerOnScreen = !g.settings.centerOnScreen; g.settings.Save();
		if (g.settings.centerOnScreen) {
			RECT wr; GetWindowRect(g.hwnd, &wr);
			int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN);
			int cx = (sw - (wr.right - wr.left)) / 2, cy = (sh - (wr.bottom - wr.top)) / 2;
			SetWindowPos(g.hwnd, nullptr, cx, cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		break;
	case 2002:DestroyWindow(g.hwnd); break;
	case 3001: {
		static COLORREF c[16] = {}; CHOOSECOLORW cc = { sizeof(cc) }; cc.hwndOwner = g.hwnd; cc.rgbResult = g.settings.textColor; cc.lpCustColors = c; cc.Flags = CC_RGBINIT | CC_FULLOPEN; if (ChooseColorW(&cc))
		{
			g.settings.textColor = cc.rgbResult; g.settings.Save(); UpdateLayered(g.hwnd);
		}
	}break;
	case 3002: {
		CHOOSEFONTW cf = { sizeof(cf) }; cf.hwndOwner = g.hwnd; LOGFONTW lf = {}; wcscpy_s(lf.lfFaceName, g.settings.fontFace); HDC dc = GetDC(g.hwnd); lf.lfHeight = -MulDiv(g.settings.fontSize, GetDeviceCaps(dc, LOGPIXELSY), 72); ReleaseDC(g.hwnd, dc); cf.lpLogFont = &lf; cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT; if (ChooseFontW(&cf))
		{
			wcscpy_s(g.settings.fontFace, lf.lfFaceName); g.settings.fontSize = cf.iPointSize / 10; if (g.settings.fontSize < 8)g.settings.fontSize = 8; if (g.settings.fontSize > 200)g.settings.fontSize = 200; g.settings.Save(); Theme_InitFont(g.settings.fontFace, g.settings.fontSize); UpdateLayered(g.hwnd);
		}
	}break;
	case 3003: {
		static COLORREF c[16] = {}; CHOOSECOLORW cc = { sizeof(cc) }; cc.hwndOwner = g.hwnd; COLORREF cur = (g.settings.promptTextColor == 0xFFFFFFFF) ? g.settings.textColor : g.settings.promptTextColor; cc.rgbResult = cur; cc.lpCustColors = c; cc.Flags = CC_RGBINIT | CC_FULLOPEN; if (ChooseColorW(&cc))
		{
			g.settings.promptTextColor = cc.rgbResult; g.settings.Save(); UpdateLayered(g.hwnd);
		}
	}break;
	case 3004: {
		CHOOSEFONTW cf = { sizeof(cf) }; cf.hwndOwner = g.hwnd; LOGFONTW lf = {}; const wchar_t* f = (g.settings.promptFontFace[0] != 0) ? g.settings.promptFontFace : g.settings.fontFace; int s = (g.settings.promptFontSize > 0) ? g.settings.promptFontSize : (int)(g.settings.fontSize * 0.28f); wcscpy_s(lf.lfFaceName, f); HDC dc = GetDC(g.hwnd); lf.lfHeight = -MulDiv(s, GetDeviceCaps(dc, LOGPIXELSY), 72); ReleaseDC(g.hwnd, dc); cf.lpLogFont = &lf; cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT; if (ChooseFontW(&cf))
		{
			wcscpy_s(g.settings.promptFontFace, lf.lfFaceName); g.settings.promptFontSize = cf.iPointSize / 10; if (g.settings.promptFontSize < 4)g.settings.promptFontSize = 4; g.settings.Save(); UpdateLayered(g.hwnd);
		}
	}break;
	}
}

static void UpdateLayered(HWND hwnd)
{
	RECT rc; GetClientRect(hwnd, &rc); int w = rc.right, h = rc.bottom; if (w <= 0 || h <= 0)return;
	HDC hdcScreen = GetDC(nullptr); HDC hdcMem = CreateCompatibleDC(hdcScreen);
	BITMAPINFO bi = {}; bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); bi.bmiHeader.biWidth = w; bi.bmiHeader.biHeight = -h; bi.bmiHeader.biPlanes = 1; bi.bmiHeader.biBitCount = 32; bi.bmiHeader.biCompression = BI_RGB;
	void* pBits = nullptr; HBITMAP hBmp = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, &pBits, nullptr, 0);
	if (!hBmp)
	{
		DeleteDC(hdcMem); ReleaseDC(nullptr, hdcScreen); return;
	}
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmp);
	Graphics gr(hdcMem); gr.SetSmoothingMode(SmoothingModeAntiAlias); gr.SetTextRenderingHint(TextRenderingHintAntiAlias);
	StringFormat sfC; sfC.SetAlignment(StringAlignmentCenter); sfC.SetLineAlignment(StringAlignmentCenter);

	if (g.editMode == 0)
	{
		int days = 0, hrs = 0, mn = 0, sc = 0; g.engine.GetDisplay(days, hrs, mn, sc); ULONGLONG rem = g.engine.RemainingMs(); bool fin = g.engine.IsFinished(); bool has = (g.engine.IsRunning() || rem > 0 || fin);
		{
			const wchar_t* ff = (g.settings.promptFontFace[0] != 0) ? g.settings.promptFontFace : g.settings.fontFace; int fs = (g.settings.promptFontSize > 0) ? g.settings.promptFontSize : (int)(g.settings.fontSize * 0.28f); Font fP(ff, (REAL)fs, FontStyleRegular, UnitPoint); COLORREF rc2 = (g.settings.promptTextColor == 0xFFFFFFFF) ? g.settings.textColor : g.settings.promptTextColor; BYTE a = (g.settings.promptTextColor == 0xFFFFFFFF) ? 140 : 255; SolidBrush br(Color(a, GetRValue(rc2), GetGValue(rc2), GetBValue(rc2))); StringFormat sf; sf.SetAlignment(StringAlignmentCenter); sf.SetLineAlignment(StringAlignmentFar); const wchar_t* p = wcslen(g.settings.promptText) ? g.settings.promptText : L"\u5012\u8ba1\u65f6"; gr.DrawString(p, -1, &fP, RectF(0, 0, (REAL)w, (REAL)(h * 0.35f)), &sf, &br);
		}
		wchar_t txt[128]; COLORREF cl = g.settings.textColor;
		if (fin)
		{
			cl = CLR_GREEN2; wcscpy_s(txt, L"\u65f6\u95f4\u5230\uff01");
		}
		else if (!has)
		{
			cl = RGB(GetRValue(cl) / 2, GetGValue(cl) / 2, GetBValue(cl) / 2); wcscpy_s(txt, L"00:00:00");
		}
		else
		{
			if (rem < 60000)cl = CLR_DANGER; else if (rem < 300000)cl = CLR_WARN; if (days > 0)swprintf_s(txt, L"%d\u5929 %02d:%02d:%02d", days, hrs, mn, sc); else swprintf_s(txt, L"%02d:%02d:%02d", hrs, mn, sc);
		}
		Font* pF = Theme_GetFont(); if (pF)
		{
			Color gc; gc.SetFromCOLORREF(cl); SolidBrush tb(gc); StringFormat sf; sf.SetAlignment(StringAlignmentCenter); sf.SetLineAlignment(StringAlignmentNear); gr.DrawString(txt, -1, pF, RectF(0, (REAL)(h * 0.33f), (REAL)w, (REAL)(h * 0.67f)), &sf, &tb);
		}
	}
	else if (g.editMode == 1)
	{
		SolidBrush bg(Color(180, 10, 10, 16)); gr.FillRectangle(&bg, 0, 0, w, h); Font fb(g.settings.fontFace, (REAL)g.settings.fontSize, FontStyleBold, UnitPoint); SolidBrush bd(Color(255, 80, 80, 100)), ba(Color(255, 137, 180, 250)); wchar_t t[128]; swprintf_s(t, L"%02d : %02d : %02d", g.editH, g.editM, g.editS); RectF tr(0, 0, (REAL)w, (REAL)(h - 24)); gr.DrawString(t, -1, &fb, tr, &sfC, &bd);
		CharacterRange rs[3] = { {0,2},{5,2},{10,2} }; StringFormat sm; sm.SetAlignment(StringAlignmentCenter); sm.SetLineAlignment(StringAlignmentCenter); sm.SetMeasurableCharacterRanges(3, rs); Region rg[3]; gr.MeasureCharacterRanges(t, -1, &fb, tr, &sm, 3, rg); RectF ar; rg[g.editField].GetBounds(&ar, &gr); gr.SetClip(ar); gr.DrawString(t, -1, &fb, tr, &sfC, &ba); gr.ResetClip(); REAL pd = ar.Width * 0.15f; gr.FillRectangle(&ba, ar.X + pd, ar.GetBottom() - 2.f, ar.Width - pd * 2, 3.f);
		Font fs(g.settings.fontFace, 10, FontStyleRegular, UnitPoint); SolidBrush dm(Color(255, 140, 140, 160)); gr.DrawString(L"\u2191\u2193 \u8c03\u503c  \u2190\u2192 \u5207\u6362  Enter \u786e\u8ba4  Esc \u53d6\u6d88", -1, &fs, RectF(0, (REAL)(h - 22), (REAL)w, 20), &sfC, &dm);
	}
	else if (g.editMode == 2)
	{
		SolidBrush bg(Color(180, 10, 10, 16)); gr.FillRectangle(&bg, 0, 0, w, h); Font fb(g.settings.fontFace, (REAL)(g.settings.fontSize * 2 / 3), FontStyleBold, UnitPoint); SolidBrush bd(Color(255, 80, 80, 100)), ba(Color(255, 137, 180, 250)); wchar_t t[128]; swprintf_s(t, L"%04d-%02d-%02d  %02d:%02d:%02d", g.editDateST.wYear, g.editDateST.wMonth, g.editDateST.wDay, g.editDateST.wHour, g.editDateST.wMinute, g.editDateST.wSecond); RectF tr(0, 0, (REAL)w, (REAL)(h - 24)); gr.DrawString(t, -1, &fb, tr, &sfC, &bd);
		CharacterRange rs[2] = { {0,10},{12,8} }; StringFormat sm; sm.SetAlignment(StringAlignmentCenter); sm.SetLineAlignment(StringAlignmentCenter); sm.SetMeasurableCharacterRanges(2, rs); Region rg[2]; gr.MeasureCharacterRanges(t, -1, &fb, tr, &sm, 2, rg); RectF ar; rg[g.editField].GetBounds(&ar, &gr); gr.SetClip(ar); gr.DrawString(t, -1, &fb, tr, &sfC, &ba); gr.ResetClip(); REAL pd = ar.Width * 0.05f; gr.FillRectangle(&ba, ar.X + pd, ar.GetBottom() - 2.f, ar.Width - pd * 2, 3.f);
		Font fs(g.settings.fontFace, 10, FontStyleRegular, UnitPoint); SolidBrush dm(Color(255, 140, 140, 160)); gr.DrawString(L"\u2191\u2193 \u8c03\u503c  \u2190\u2192 \u5207\u6362  Enter \u786e\u8ba4  Esc \u53d6\u6d88", -1, &fs, RectF(0, (REAL)(h - 22), (REAL)w, 20), &sfC, &dm);
	}
	else
	{
		SolidBrush bg(Color(180, 10, 10, 16)); gr.FillRectangle(&bg, 0, 0, w, h); Font fh(g.settings.fontFace, (REAL)(g.settings.fontSize * 0.2f), FontStyleRegular, UnitPoint); SolidBrush bh(Color(200, 160, 160, 180)); gr.DrawString(L"\u8f93\u5165\u63d0\u793a\u6587\u672c\uff08Enter \u4fdd\u5b58 / Esc \u53d6\u6d88\uff09:", -1, &fh, RectF(0, (REAL)(h * 0.05f), (REAL)w, (REAL)(h * 0.15f)), &sfC, &bh);
		Font fe(g.settings.fontFace, (REAL)(g.settings.fontSize * 0.35f), FontStyleBold, UnitPoint); SolidBrush be(Color(255, 255, 255, 255)); wchar_t d[160]; swprintf_s(d, L"%s|", g.editPrompt); sfC.SetLineAlignment(StringAlignmentCenter); gr.DrawString(d, -1, &fe, RectF(0, (REAL)(h * 0.15f), (REAL)w, (REAL)(h * 0.55f)), &sfC, &be);
		Font fn(g.settings.fontFace, 9, FontStyleRegular, UnitPoint); gr.DrawString(L"Backspace \u5220\u9664 | Esc \u53d6\u6d88 | Enter \u786e\u8ba4", -1, &fn, RectF(0, (REAL)(h * 0.72f), (REAL)w, (REAL)(h * 0.18f)), &sfC, &bh);
	}

	BLENDFUNCTION blend = { AC_SRC_OVER,0,255,AC_SRC_ALPHA }; SIZE sizeWnd = { w,h }; POINT ptSrc = { 0,0 }, ptDst = { rc.left,rc.top }; ClientToScreen(hwnd, &ptDst); UpdateLayeredWindow(hwnd, hdcScreen, &ptDst, &sizeWnd, hdcMem, &ptSrc, 0, &blend, ULW_ALPHA);
	SelectObject(hdcMem, hOldBmp); DeleteObject(hBmp); DeleteDC(hdcMem); ReleaseDC(nullptr, hdcScreen);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
		g.hwnd = hwnd; g.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_MAIN_ICON)); g.settings.Load(); SetAutoStart(g.settings.autoStart); g.inputMode = g.settings.inputMode; g.editH = g.settings.lastHours; g.editM = g.settings.lastMinutes; g.editS = g.settings.lastSeconds; Theme_InitFont(g.settings.fontFace, g.settings.fontSize); SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		if (g.settings.centerOnScreen) { RECT wr; GetWindowRect(hwnd, &wr); int sw = GetSystemMetrics(SM_CXSCREEN), sh = GetSystemMetrics(SM_CYSCREEN); int cx = (sw - (wr.right - wr.left)) / 2, cy = (sh - (wr.bottom - wr.top)) / 2; SetWindowPos(hwnd, nullptr, cx, cy, 0, 0, SWP_NOSIZE | SWP_NOZORDER); }
		g.tray.Add(hwnd, g.hIcon); SetTimer(hwnd, IDT_COUNTDOWN, TIMER_INTERVAL, nullptr); PostMessageW(hwnd, WM_APP, 0, 0); return 0;
	case WM_APP:AutoRestoreCountdown(); return 0;
	case WM_SIZE:UpdateLayered(hwnd); return 0;
	case WM_PAINT: {
		PAINTSTRUCT ps; BeginPaint(hwnd, &ps); EndPaint(hwnd, &ps); return 0;
	}
	case WM_ERASEBKGND:return 1;
	case WM_CHAR:if (g.editMode == 3 && wp >= 32 && wp != 127)
	{
		size_t len = wcslen(g.editPrompt); if (wp == '\b')
		{
			if (len > 0)g.editPrompt[len - 1] = L'\0';
		}
		else if (len < 126)
		{
			g.editPrompt[len] = (wchar_t)wp; g.editPrompt[len + 1] = L'\0';
		}UpdateLayered(hwnd); return 0;
	}break;
	case WM_KEYDOWN:
		if (g.editMode == 1)
		{
			switch (wp)
			{
			case VK_UP:AdjMin(1); break; case VK_DOWN:AdjMin(-1); break; case VK_LEFT:g.editField = (g.editField + 2) % 3; break; case VK_RIGHT:g.editField = (g.editField + 1) % 3; break; case VK_RETURN:CommitEdit(); break; case VK_ESCAPE:CancelEdit(); break;
			}UpdateLayered(hwnd);
		}
		else if (g.editMode == 2)
		{
			switch (wp)
			{
			case VK_UP:AdjDate(1); break; case VK_DOWN:AdjDate(-1); break; case VK_LEFT:g.editField = 0; break; case VK_RIGHT:g.editField = 1; break; case VK_RETURN:CommitEdit(); break; case VK_ESCAPE:CancelEdit(); break;
			}UpdateLayered(hwnd);
		}
		else if (g.editMode == 3)
		{
			if (wp == VK_RETURN)
			{
				CommitEdit(); return 0;
			}if (wp == VK_ESCAPE)
			{
				CancelEdit(); return 0;
			}if (wp == VK_DELETE)
			{
				size_t len = wcslen(g.editPrompt); if (len > 0)g.editPrompt[len - 1] = L'\0'; UpdateLayered(hwnd);
			}
		}
		else
		{
			switch (wp)
			{
			case VK_SPACE:PauseResume(); break; case'R':case'r':DoReset(); break; case'T':case't':g.inputMode = 0; BeginEditDuration(); break; case'D':case'd':g.inputMode = 1; BeginEditDate(); break; case'P':case'p':BeginEditPrompt(); break; case VK_OEM_PLUS:case VK_ADD: {
				int s = g.settings.fontSize + 8; if (s > 200)s = 200; g.settings.fontSize = s; g.settings.Save(); Theme_InitFont(g.settings.fontFace, s); UpdateLayered(hwnd);
			}break; case VK_OEM_MINUS:case VK_SUBTRACT: {
				int s = g.settings.fontSize - 8; if (s < 8)s = 8; g.settings.fontSize = s; g.settings.Save(); Theme_InitFont(g.settings.fontFace, s); UpdateLayered(hwnd);
			}break;
			}
		}return 0;
	case WM_LBUTTONDOWN:g.dragging = true; g.dragX = (int)(short)LOWORD(lp); g.dragY = (int)(short)HIWORD(lp); SetCapture(hwnd); return 0;
	case WM_MOUSEMOVE:if (g.dragging)
	{
		POINT pt; GetCursorPos(&pt); SetWindowPos(hwnd, nullptr, pt.x - g.dragX, pt.y - g.dragY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}return 0;
	case WM_LBUTTONUP:if (g.dragging)
	{
		g.dragging = false; ReleaseCapture();
	}return 0;
	case WM_LBUTTONDBLCLK:if (!g.dragging && !g.engine.IsRunning())
	{
		if (g.inputMode == 0)BeginEditDuration(); else BeginEditDate(); UpdateLayered(hwnd);
	}return 0;
	case WM_RBUTTONUP: {
		POINT pt; GetCursorPos(&pt); HMENU m = BuildContextMenu(); SetForegroundWindow(hwnd); WORD cmd = (WORD)TrackPopupMenu(m, TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr); if (cmd)HandleMenuCmd(cmd); DestroyMenu(m);
	}return 0;
	case WM_TIMER:if (wp == IDT_COUNTDOWN && g.engine.IsRunning())
	{
		g.engine.Tick(); int d, hr, mn, sc; g.engine.GetDisplay(d, hr, mn, sc); wchar_t tip[128]; if (d > 0)swprintf_s(tip, L"%s: %d\u5929 %02d:%02d:%02d", g.settings.promptText, d, hr, mn, sc); else swprintf_s(tip, L"%s: %02d:%02d:%02d", g.settings.promptText, hr, mn, sc); g.tray.UpdateTip(tip); if (g.engine.IsFinished())OnFinished(); UpdateLayered(hwnd);
	}return 0;
	case WM_TRAYICON:if (LOWORD(lp) == WM_LBUTTONDBLCLK)
	{
		ShowWindow(hwnd, SW_SHOW); SetForegroundWindow(hwnd);
	}
					else if (LOWORD(lp) == WM_RBUTTONUP)
	{
		g.tray.ShowContextMenu(hwnd);
	}return 0;
	case WM_COMMAND:if (LOWORD(wp) == IDM_TRAY_RESTORE)
	{
		ShowWindow(hwnd, SW_SHOW); SetForegroundWindow(hwnd);
	}
				   else if (LOWORD(wp) == IDM_TRAY_EXIT)DestroyWindow(hwnd); return 0;
	case WM_CLOSE:if (g.settings.minimizeToTray)
	{
		ShowWindow(hwnd, SW_HIDE); return 0;
	}DestroyWindow(hwnd); return 0;
	case WM_DESTROY: {
		RECT r; GetWindowRect(hwnd, &r); g.settings.winX = r.left; g.settings.winY = r.top; g.settings.winW = r.right - r.left; g.settings.winH = r.bottom - r.top; g.settings.wasRunning = g.engine.IsRunning(); if (g.engine.IsRunning())
		{
			g.engine.Pause(); g.settings.savedRemainingMs = g.engine.RemainingMs();
		}g.settings.Save(); KillTimer(hwnd, IDT_COUNTDOWN); g.tray.Remove(); Theme_CleanupFont(); PostQuitMessage(0); return 0;
	}
	case WM_QUERYENDSESSION:g.settings.wasRunning = g.engine.IsRunning(); if (g.engine.IsRunning())
	{
		g.engine.Pause(); g.settings.savedRemainingMs = g.engine.RemainingMs();
	}g.settings.Save(); return TRUE;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int nCmd)
{
	INITCOMMONCONTROLSEX icc = { sizeof(icc),ICC_STANDARD_CLASSES }; InitCommonControlsEx(&icc); ULONG_PTR gdiTok = 0; GdiplusStartupInput gdiIn; GdiplusStartup(&gdiTok, &gdiIn, nullptr);
	WNDCLASSEXW wc = {}; wc.cbSize = sizeof(wc); wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS; wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.hCursor = LoadCursorW(nullptr, IDC_ARROW); wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); wc.lpszClassName = L"CountdownOverlay"; wc.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_MAIN_ICON)); wc.hIconSm = wc.hIcon; RegisterClassExW(&wc);
	g.settings.Load(); g.editH = g.settings.lastHours; g.editM = g.settings.lastMinutes; g.editS = g.settings.lastSeconds;
	HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"CountdownOverlay", L"Countdown", WS_POPUP, g.settings.winX, g.settings.winY, g.settings.winW, g.settings.winH, nullptr, nullptr, hInst, nullptr);
	if (!hwnd)
	{
		GdiplusShutdown(gdiTok); return 1;
	}UpdateLayered(hwnd); ShowWindow(hwnd, nCmd);
	MSG msg; while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg); DispatchMessageW(&msg);
	}GdiplusShutdown(gdiTok); return(int)msg.wParam;
}
