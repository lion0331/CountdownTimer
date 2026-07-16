// Common.h - shared header
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <objidl.h>
#include <gdiplus.h>
#include <string>
#include <cstdio>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comdlg32.lib")

#include "resource.h"

constexpr const wchar_t* APP_NAME = L"CountdownTimer";
constexpr const wchar_t* APP_TITLE = L"Countdown";
constexpr const wchar_t* REG_KEY = L"Software\\CountdownTimer";
constexpr const wchar_t* DEFAULT_FONT = L"Segoe UI";
constexpr int  TIMER_INTERVAL = 200;
constexpr int  WIN_W = 460, WIN_H = 180;

constexpr COLORREF CLR_BG = RGB(0x1E, 0x1E, 0x2E);
constexpr COLORREF CLR_BG_LIGHT = RGB(0x31, 0x32, 0x44);
constexpr COLORREF CLR_TEXT = RGB(0xCD, 0xD6, 0xF4);
constexpr COLORREF CLR_ACCENT = RGB(0x89, 0xB4, 0xFA);
constexpr COLORREF CLR_GREEN2 = RGB(0xA6, 0xE3, 0xA1);
constexpr COLORREF CLR_WARN = RGB(0xFA, 0xB3, 0x87);
constexpr COLORREF CLR_DANGER = RGB(0xF3, 0x8B, 0xA8);

constexpr UINT WM_TRAYICON = WM_APP + 1;

// AppSettings struct - all fields inline
struct AppSettings
{
	wchar_t fontFace[64] = L"Segoe UI";
	int     fontSize = 72;
	COLORREF textColor = CLR_TEXT;
	wchar_t promptFontFace[64] = L"";
	int     promptFontSize = 0;
	COLORREF promptTextColor = 0xFFFFFFFF;
	wchar_t soundPath[MAX_PATH] = L"";
	bool    useCustomSound = false;
	bool    alwaysOnTop = false;
	bool    autoStart          = false;
	bool    centerOnScreen     = false;
	bool    minimizeToTray     = true;
	int     winX = CW_USEDEFAULT, winY = CW_USEDEFAULT;
	int     winW = WIN_W, winH = WIN_H;
	wchar_t promptText[128];
	int     lastHours = 0, lastMinutes = 5, lastSeconds = 0;
	int     inputMode = 0;
	WORD    lastDateYear = 0, lastDateMonth = 0, lastDateDay = 0;
	WORD    lastTimeHour = 0, lastTimeMin = 0, lastTimeSec = 0;
	bool      wasRunning = false;
	ULONGLONG savedRemainingMs = 0;

	void Load();
	void Save() const;
};

struct CountdownEngine;