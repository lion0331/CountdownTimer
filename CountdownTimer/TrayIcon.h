// TrayIcon.h - 系统托盘管理
#pragma once
#include "Common.h"

struct TrayIcon
{
    NOTIFYICONDATAW nid = {};

    void Add(HWND hwnd, HICON hIcon);
    void UpdateTip(const wchar_t* tip);
    void ShowBalloon(const wchar_t* title, const wchar_t* info);
    void Remove();
    void ShowContextMenu(HWND hwnd);
};
