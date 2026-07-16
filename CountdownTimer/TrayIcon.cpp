// TrayIcon.cpp - 托盘实现
#include "TrayIcon.h"

void TrayIcon::Add(HWND hwnd, HICON hIcon)
{
    nid.cbSize           = sizeof(NOTIFYICONDATAW);
    nid.hWnd             = hwnd;
    nid.uID              = 1;
    nid.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon            = hIcon;
    wcscpy_s(nid.szTip, L"\u5012\u8ba1\u65f6 - \u672a\u5f00\u59cb");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

void TrayIcon::UpdateTip(const wchar_t* tip)
{
    wcscpy_s(nid.szTip, tip);
    nid.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void TrayIcon::ShowBalloon(const wchar_t* title, const wchar_t* info)
{
    nid.uFlags = NIF_INFO;
    wcscpy_s(nid.szInfo, info);
    wcscpy_s(nid.szInfoTitle, title);
    nid.dwInfoFlags = NIIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void TrayIcon::Remove()
{
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

void TrayIcon::ShowContextMenu(HWND hwnd)
{
    POINT pt; GetCursorPos(&pt);
    HMENU hMenu = CreatePopupMenu();
    InsertMenuW(hMenu, 0, MF_BYPOSITION | MF_STRING, IDM_TRAY_RESTORE, L"\u6062\u590D\u7A97\u53E3");
    InsertMenuW(hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, nullptr);
    InsertMenuW(hMenu, 2, MF_BYPOSITION | MF_STRING, IDM_TRAY_EXIT,    L"\u9000\u51FA");

    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, nullptr);
    PostMessageW(hwnd, WM_NULL, 0, 0);
    DestroyMenu(hMenu);
}
