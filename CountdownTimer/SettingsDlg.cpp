// SettingsDlg.cpp - 设置对话框实现
#include "SettingsDlg.h"

static COLORREF g_curColor = CLR_TEXT;

// 颜色预览控件子类化 — 显式 CALLBACK 确保 x86 下 stdcall 调用约定
static LRESULT CALLBACK ColorPreviewProc(HWND hw, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
	if (msg == WM_PAINT)
	{
		PAINTSTRUCT ps; HDC hdc = BeginPaint(hw, &ps);
		RECT rc; GetClientRect(hw, &rc);
		HBRUSH br = CreateSolidBrush(g_curColor);
		FillRect(hdc, &rc, br);
		DeleteObject(br);
		EndPaint(hw, &ps);
		return 0;
	}
	return DefSubclassProc(hw, msg, wp, lp);
}

// 字体枚举回调 — 显式 CALLBACK 确保 x86 下 stdcall 调用约定
static int CALLBACK EnumFontProc(const LOGFONTW* plf, const TEXTMETRICW*, DWORD, LPARAM lp)
{
	if (plf->lfFaceName[0] != L'@')
		SendMessageW((HWND)lp, CB_ADDSTRING, 0, (LPARAM)plf->lfFaceName);
	return 1;
}

static LRESULT CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static AppSettings* pSettings = nullptr;

	switch (msg)
	{
	case WM_INITDIALOG: {
		pSettings = (AppSettings*)lParam;
		g_curColor = pSettings->textColor;

		// 列举系统字体
		HWND hCombo = GetDlgItem(hDlg, IDC_FONT_FACE_COMBO);
		HDC hdc = GetDC(hDlg);
		LOGFONTW lf = {}; lf.lfCharSet = DEFAULT_CHARSET;
		EnumFontFamiliesExW(hdc, &lf, EnumFontProc, (LPARAM)hCombo, 0);
		ReleaseDC(hDlg, hdc);

		SetDlgItemTextW(hDlg, IDC_FONT_FACE_COMBO, pSettings->fontFace);
		SetDlgItemInt(hDlg, IDC_FONT_SIZE_EDIT, pSettings->fontSize, FALSE);
		SendDlgItemMessageW(hDlg, IDC_FONT_SIZE_SPIN, UDM_SETRANGE, 0, MAKELPARAM(200, 8));
		SetDlgItemTextW(hDlg, IDC_SOUND_PATH_EDIT, pSettings->soundPath);

		if (pSettings->useCustomSound && wcslen(pSettings->soundPath) > 0)
		{
			CheckDlgButton(hDlg, IDC_SOUND_CUSTOM_RADIO, BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_PATH_EDIT), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_BROWSE_BTN), TRUE);
		}
		else
		{
			CheckDlgButton(hDlg, IDC_SOUND_DEFAULT_RADIO, BST_CHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_PATH_EDIT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_BROWSE_BTN), FALSE);
		}

		CheckDlgButton(hDlg, IDC_TOPMOST_CHECK, pSettings->alwaysOnTop ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_TRAY_CHECK, pSettings->minimizeToTray ? BST_CHECKED : BST_UNCHECKED);

		// 颜色预览子类化 — 使用显式 CALLBACK 函数
		SetWindowSubclass(GetDlgItem(hDlg, IDC_COLOR_PREVIEW), ColorPreviewProc, 0, 0);
		return TRUE;
	}

	case WM_COMMAND: {
		switch (LOWORD(wParam))
		{
		case IDC_SOUND_DEFAULT_RADIO:
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_PATH_EDIT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_BROWSE_BTN), FALSE);
			break;
		case IDC_SOUND_CUSTOM_RADIO:
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_PATH_EDIT), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_SOUND_BROWSE_BTN), TRUE);
			break;
		case IDC_SOUND_BROWSE_BTN: {
			wchar_t path[MAX_PATH] = {};
			OPENFILENAMEW ofn = {};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = L"音频文件\0*.wav;*.mp3\0WAV 文件\0*.wav\0所有文件\0*.*\0";
			ofn.lpstrFile = path;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if (GetOpenFileNameW(&ofn))
				SetDlgItemTextW(hDlg, IDC_SOUND_PATH_EDIT, path);
			break;
		}
		case IDC_SOUND_TEST_BTN: {
			wchar_t path[MAX_PATH];
			GetDlgItemTextW(hDlg, IDC_SOUND_PATH_EDIT, path, MAX_PATH);
			if (IsDlgButtonChecked(hDlg, IDC_SOUND_CUSTOM_RADIO) && wcslen(path) > 0)
				PlaySoundW(path, nullptr, SND_FILENAME | SND_ASYNC);
			else
				MessageBeep(MB_ICONINFORMATION);
			break;
		}
		case IDC_COLOR_CHOOSE_BTN: {
			CHOOSECOLORW cc = {};
			static COLORREF cust[16] = {};
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = hDlg;
			cc.rgbResult = g_curColor;
			cc.lpCustColors = cust;
			cc.Flags = CC_RGBINIT | CC_FULLOPEN;
			if (ChooseColorW(&cc))
			{
				g_curColor = cc.rgbResult;
				InvalidateRect(GetDlgItem(hDlg, IDC_COLOR_PREVIEW), nullptr, FALSE);
			}
			break;
		}
		case IDC_OK_BTN: {
			GetDlgItemTextW(hDlg, IDC_FONT_FACE_COMBO, pSettings->fontFace, 64);
			pSettings->fontSize = GetDlgItemInt(hDlg, IDC_FONT_SIZE_EDIT, nullptr, FALSE);
			if (pSettings->fontSize < 8)   pSettings->fontSize = 8;
			if (pSettings->fontSize > 200) pSettings->fontSize = 200;

			pSettings->textColor = g_curColor;

			pSettings->useCustomSound = (IsDlgButtonChecked(hDlg, IDC_SOUND_CUSTOM_RADIO) == BST_CHECKED);
			GetDlgItemTextW(hDlg, IDC_SOUND_PATH_EDIT, pSettings->soundPath, MAX_PATH);

			pSettings->alwaysOnTop = (IsDlgButtonChecked(hDlg, IDC_TOPMOST_CHECK) == BST_CHECKED);
			pSettings->minimizeToTray = (IsDlgButtonChecked(hDlg, IDC_TRAY_CHECK) == BST_CHECKED);

			pSettings->Save();
			EndDialog(hDlg, IDOK);
			break;
		}
		case IDC_CANCEL_BTN:
			EndDialog(hDlg, IDCANCEL);
			break;
		}
		break;
	}
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;
	}
	return FALSE;
}

bool SettingsDlg_Show(HWND hwndOwner, AppSettings& settings)
{
	return DialogBoxParamW(GetModuleHandleW(nullptr),
		MAKEINTRESOURCEW(IDD_SETTINGS_DLG), hwndOwner, DlgProc,
		(LPARAM)&settings) == IDOK;
}
