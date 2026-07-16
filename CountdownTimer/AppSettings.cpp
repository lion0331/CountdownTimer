// AppSettings.cpp - 设置持久化实现
#include "Common.h"

void AppSettings::Load()
{
	HKEY hKey;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return;

	DWORD sz;
	sz = sizeof(fontFace);         RegQueryValueExW(hKey, L"FontFace", nullptr, nullptr, (LPBYTE)fontFace, &sz);
	sz = sizeof(fontSize);         RegQueryValueExW(hKey, L"FontSize", nullptr, nullptr, (LPBYTE)&fontSize, &sz);
	sz = sizeof(textColor);        RegQueryValueExW(hKey, L"TextColor", nullptr, nullptr, (LPBYTE)&textColor, &sz);
	sz = sizeof(promptFontFace);   RegQueryValueExW(hKey, L"PromptFontFace", nullptr, nullptr, (LPBYTE)promptFontFace, &sz);
	sz = sizeof(promptFontSize);   RegQueryValueExW(hKey, L"PromptFontSize", nullptr, nullptr, (LPBYTE)&promptFontSize, &sz);
	sz = sizeof(promptTextColor);  RegQueryValueExW(hKey, L"PromptTextColor", nullptr, nullptr, (LPBYTE)&promptTextColor, &sz);
	sz = sizeof(soundPath);        RegQueryValueExW(hKey, L"SoundPath", nullptr, nullptr, (LPBYTE)soundPath, &sz);
	sz = sizeof(promptText);       RegQueryValueExW(hKey, L"PromptText", nullptr, nullptr, (LPBYTE)promptText, &sz);
	sz = sizeof(DWORD);            RegQueryValueExW(hKey, L"UseCustomSound", nullptr, nullptr, (LPBYTE)&useCustomSound, &sz);
	sz = sizeof(DWORD);            RegQueryValueExW(hKey, L"AlwaysOnTop", nullptr, nullptr, (LPBYTE)&alwaysOnTop, &sz);
	sz = sizeof(DWORD);            RegQueryValueExW(hKey, L"AutoStart", nullptr, nullptr, (LPBYTE)&autoStart, &sz);
	sz = sizeof(DWORD);            RegQueryValueExW(hKey, L"MinimizeToTray", nullptr, nullptr, (LPBYTE)&minimizeToTray, &sz);
	sz = sizeof(winX);             RegQueryValueExW(hKey, L"WinX", nullptr, nullptr, (LPBYTE)&winX, &sz);
	sz = sizeof(winY);             RegQueryValueExW(hKey, L"WinY", nullptr, nullptr, (LPBYTE)&winY, &sz);
	sz = sizeof(winW);             RegQueryValueExW(hKey, L"WinW", nullptr, nullptr, (LPBYTE)&winW, &sz);
	sz = sizeof(winH);             RegQueryValueExW(hKey, L"WinH", nullptr, nullptr, (LPBYTE)&winH, &sz);
	sz = sizeof(lastHours);        RegQueryValueExW(hKey, L"LastHours", nullptr, nullptr, (LPBYTE)&lastHours, &sz);
	sz = sizeof(lastMinutes);      RegQueryValueExW(hKey, L"LastMinutes", nullptr, nullptr, (LPBYTE)&lastMinutes, &sz);
	sz = sizeof(lastSeconds);      RegQueryValueExW(hKey, L"LastSeconds", nullptr, nullptr, (LPBYTE)&lastSeconds, &sz);
	sz = sizeof(inputMode);        RegQueryValueExW(hKey, L"InputMode", nullptr, nullptr, (LPBYTE)&inputMode, &sz);
	sz = sizeof(lastDateYear);     RegQueryValueExW(hKey, L"LastDateYear", nullptr, nullptr, (LPBYTE)&lastDateYear, &sz);
	sz = sizeof(lastDateMonth);    RegQueryValueExW(hKey, L"LastDateMonth", nullptr, nullptr, (LPBYTE)&lastDateMonth, &sz);
	sz = sizeof(lastDateDay);      RegQueryValueExW(hKey, L"LastDateDay", nullptr, nullptr, (LPBYTE)&lastDateDay, &sz);
	sz = sizeof(lastTimeHour);     RegQueryValueExW(hKey, L"LastTimeHour", nullptr, nullptr, (LPBYTE)&lastTimeHour, &sz);
	sz = sizeof(lastTimeMin);      RegQueryValueExW(hKey, L"LastTimeMin", nullptr, nullptr, (LPBYTE)&lastTimeMin, &sz);
	sz = sizeof(lastTimeSec);      RegQueryValueExW(hKey, L"LastTimeSec", nullptr, nullptr, (LPBYTE)&lastTimeSec, &sz);
	sz = sizeof(DWORD);            RegQueryValueExW(hKey, L"WasRunning", nullptr, nullptr, (LPBYTE)&wasRunning, &sz);
	sz = sizeof(savedRemainingMs); RegQueryValueExW(hKey, L"SavedRemainingMs", nullptr, nullptr, (LPBYTE)&savedRemainingMs, &sz);
	RegCloseKey(hKey);

	if (textColor == 0 || textColor == 0xFFFFFFFF) textColor = CLR_TEXT;
	if (promptText[0] == 0)
	{
		const wchar_t* def = L"\u5012\u8ba1\u65f6"; wcscpy_s(promptText, def);
	}
}

void AppSettings::Save() const
{
	HKEY hKey;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY, 0, nullptr,
		REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS)
		return;

	DWORD dwUseCustomSound = useCustomSound ? 1 : 0;
	DWORD dwAlwaysOnTop = alwaysOnTop ? 1 : 0;
	DWORD dwAutoStart = autoStart ? 1 : 0;
	DWORD dwMinimizeToTray = minimizeToTray ? 1 : 0;
	DWORD dwWasRunning = wasRunning ? 1 : 0;

	RegSetValueExW(hKey, L"FontFace", 0, REG_SZ, (LPBYTE)fontFace, (DWORD)((wcslen(fontFace) + 1) * sizeof(wchar_t)));
	RegSetValueExW(hKey, L"FontSize", 0, REG_DWORD, (LPBYTE)&fontSize, sizeof(int));
	RegSetValueExW(hKey, L"TextColor", 0, REG_DWORD, (LPBYTE)&textColor, sizeof(COLORREF));
	RegSetValueExW(hKey, L"PromptFontFace", 0, REG_SZ, (LPBYTE)promptFontFace, (DWORD)((wcslen(promptFontFace) + 1) * sizeof(wchar_t)));
	RegSetValueExW(hKey, L"PromptFontSize", 0, REG_DWORD, (LPBYTE)&promptFontSize, sizeof(int));
	RegSetValueExW(hKey, L"PromptTextColor", 0, REG_DWORD, (LPBYTE)&promptTextColor, sizeof(COLORREF));
	RegSetValueExW(hKey, L"SoundPath", 0, REG_SZ, (LPBYTE)soundPath, (DWORD)((wcslen(soundPath) + 1) * sizeof(wchar_t)));
	RegSetValueExW(hKey, L"PromptText", 0, REG_SZ, (LPBYTE)promptText, (DWORD)((wcslen(promptText) + 1) * sizeof(wchar_t)));
	RegSetValueExW(hKey, L"UseCustomSound", 0, REG_DWORD, (LPBYTE)&dwUseCustomSound, sizeof(DWORD));
	RegSetValueExW(hKey, L"AlwaysOnTop", 0, REG_DWORD, (LPBYTE)&dwAlwaysOnTop, sizeof(DWORD));
	RegSetValueExW(hKey, L"AutoStart", 0, REG_DWORD, (LPBYTE)&dwAutoStart, sizeof(DWORD));
	RegSetValueExW(hKey, L"MinimizeToTray", 0, REG_DWORD, (LPBYTE)&dwMinimizeToTray, sizeof(DWORD));
	RegSetValueExW(hKey, L"WinX", 0, REG_DWORD, (LPBYTE)&winX, sizeof(int));
	RegSetValueExW(hKey, L"WinY", 0, REG_DWORD, (LPBYTE)&winY, sizeof(int));
	RegSetValueExW(hKey, L"WinW", 0, REG_DWORD, (LPBYTE)&winW, sizeof(int));
	RegSetValueExW(hKey, L"WinH", 0, REG_DWORD, (LPBYTE)&winH, sizeof(int));
	RegSetValueExW(hKey, L"LastHours", 0, REG_DWORD, (LPBYTE)&lastHours, sizeof(int));
	RegSetValueExW(hKey, L"LastMinutes", 0, REG_DWORD, (LPBYTE)&lastMinutes, sizeof(int));
	RegSetValueExW(hKey, L"LastSeconds", 0, REG_DWORD, (LPBYTE)&lastSeconds, sizeof(int));
	RegSetValueExW(hKey, L"InputMode", 0, REG_DWORD, (LPBYTE)&inputMode, sizeof(int));
	RegSetValueExW(hKey, L"LastDateYear", 0, REG_DWORD, (LPBYTE)&lastDateYear, sizeof(WORD));
	RegSetValueExW(hKey, L"LastDateMonth", 0, REG_DWORD, (LPBYTE)&lastDateMonth, sizeof(WORD));
	RegSetValueExW(hKey, L"LastDateDay", 0, REG_DWORD, (LPBYTE)&lastDateDay, sizeof(WORD));
	RegSetValueExW(hKey, L"LastTimeHour", 0, REG_DWORD, (LPBYTE)&lastTimeHour, sizeof(WORD));
	RegSetValueExW(hKey, L"LastTimeMin", 0, REG_DWORD, (LPBYTE)&lastTimeMin, sizeof(WORD));
	RegSetValueExW(hKey, L"LastTimeSec", 0, REG_DWORD, (LPBYTE)&lastTimeSec, sizeof(WORD));
	RegSetValueExW(hKey, L"WasRunning", 0, REG_DWORD, (LPBYTE)&dwWasRunning, sizeof(DWORD));
	RegSetValueExW(hKey, L"SavedRemainingMs", 0, REG_QWORD, (LPBYTE)&savedRemainingMs, sizeof(ULONGLONG));
	RegCloseKey(hKey);
}
