#include<Windows.h>
#include<stack>
#include"resource.h"
#include"about_window.h"
#include <mmsystem.h>
#include<windowsx.h>
HINSTANCE g_hInst;
#define COLOR_TRANSPARENT   RGB(122,122,0)
#pragma comment(lib,"Winmm.lib")
#if defined(_WIN64)
#define GCL_HCURSOR (-12)
#endif
DWORD WINAPI WaitFinish(LPVOID lpParam)
{
	HWND hwnd = (HWND)lpParam;
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	while (true)
	{
		WaitForSingleObject(hEvent, INFINITE);
		HDC hdc = GetDC(hwnd);
		HBITMAP bmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));
		HDC hMemDC = CreateCompatibleDC(hdc);
		SelectObject(hMemDC, bmp);
		BITMAP bmInfo;
		int buflen = GetObject(bmp, sizeof(bmInfo), &bmInfo);
		StretchBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
		ReleaseDC(hwnd, hdc);
		DeleteObject(bmp);
		DeleteDC(hMemDC);
	}
	return 0;
}
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrev);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	g_hInst = hInst;
	WCHAR szClassName[] = L"成果截屏";
	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = 0;
	wndClassEx.lpfnWndProc = MyWindowProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = g_hInst;
	wndClassEx.hIcon = (HICON)LoadImage(g_hInst,MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground =CreateSolidBrush(COLOR_TRANSPARENT);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = szClassName;
	wndClassEx.hIconSm = NULL;
	if (!RegisterClassEx(&wndClassEx))
	{
		return -1;
	}
	HWND hwnd = CreateWindowEx(/*WS_EX_TOPMOST |*/ WS_EX_LAYERED,
	//HWND hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED,
		szClassName,
		L"成果截屏",
		WS_POPUP,
		0, 0, 0, 0, NULL, NULL, g_hInst, NULL);
	SetLayeredWindowAttributes(hwnd, COLOR_TRANSPARENT, 0, LWA_COLORKEY);
	if (hwnd == NULL)
	{
		return -2;
	}
	CreateThread(NULL, 0, WaitFinish, hwnd, 0, NULL);
	ShowWindow(hwnd, SW_NORMAL);
	UpdateWindow(hwnd);
	MSG msg;
	HACCEL hAccTable = LoadAccelerators(g_hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));
	while (GetMessage(&msg,NULL,0,0))
	{
		if (!TranslateAccelerator(hwnd,hAccTable,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
enum class SHAPE
{
	PEN,
	RECTANGLE,
	CIRCLE
};
#define WM_ICONMESSAGE WM_USER+1
#define ID_HOTKEY_SHOW   100
#define ID_HOTKEY_QUIT     200
NOTIFYICONDATA notify_i_data;
LRESULT CALLBACK MyWindowProc(
	HWND   hwnd,
	UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static COLORREF pen_color = RGB(0, 0, 0);
	static int pen_widths[] = { 1,5,7,9,10,12,14,15,20,25,30,35,40,45 };
	//static int cursor_widths[] = { 10,14,18,22,26,30,34,38,42,46,50,54 };
	static int pen_widths_index = 3;
	static int width, height;
	static WORD old_x, old_y;
	static POINT ltpoint;
	static SHAPE shape = SHAPE::PEN;
	static ::std::stack<COLORREF> stack;
	static HBITMAP old_bmp = NULL;
	static bool is_checked = false;
	static bool is_sounds_open = true;
	static bool is_window_hidden = false;
	static bool is_erase = false;
	switch (uMsg)
	{
	case WM_CREATE:
	{
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(hwnd, 0, 0, width, height, FALSE);
		RegisterHotKey(hwnd, ID_HOTKEY_SHOW, MOD_ALT | MOD_CONTROL, 0x58);
		RegisterHotKey(hwnd, ID_HOTKEY_QUIT, MOD_ALT | MOD_CONTROL, 0x51);
		notify_i_data.cbSize = sizeof(NOTIFYICONDATA);
		notify_i_data.hWnd = hwnd;
		notify_i_data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO | NIF_SHOWTIP;
		notify_i_data.uCallbackMessage = WM_ICONMESSAGE;
		notify_i_data.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON4));
		PCTSTR szNotify = TEXT("成果截屏成功启动!");
		_tcsncpy_s(notify_i_data.szTip, _tcslen(szNotify) + 1, szNotify, sizeof(notify_i_data.szTip));
		PCTSTR szInfo = TEXT("成果,我还在原地");
		_tcsncpy_s(notify_i_data.szInfo, _tcslen(szInfo) + 1, szInfo, sizeof(notify_i_data.szInfo));
		PCTSTR szInfoTitle = TEXT("系统通知");
		_tcsncpy_s(notify_i_data.szInfoTitle, _tcslen(szInfoTitle) + 1, szInfoTitle, sizeof(notify_i_data.szInfoTitle));
		notify_i_data.dwInfoFlags = NIIF_USER;
		notify_i_data.hBalloonIcon = NULL;
		notify_i_data.uID = 520;
		notify_i_data.uVersion = NOTIFYICON_VERSION_4;
		Shell_NotifyIcon(NIM_ADD, &notify_i_data);
		Shell_NotifyIcon(NIM_SETVERSION, &notify_i_data);
		break;
	}
	case WM_HOTKEY:
	{
		if (wParam == ID_HOTKEY_SHOW)
		{
			if (is_window_hidden)
			{
				ShowWindow(hwnd, SW_SHOWNORMAL);
				hdc = GetDC(hwnd);
				HDC hMemDC = CreateCompatibleDC(hdc);
				SelectObject(hMemDC, old_bmp);
				BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);
				ReleaseDC(hwnd, hdc);
				is_window_hidden = false;
			}
		}
		if (wParam == ID_HOTKEY_QUIT)
		{
			Shell_NotifyIcon(NIM_DELETE, &notify_i_data);
			PostQuitMessage(0);
		}
		break;
	}
	case WM_ICONMESSAGE:
	{
		if (LOWORD(lParam) == WM_CONTEXTMENU)
		{
			POINT pt;
			GetCursorPos(&pt);
			HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU2));
			HMENU hSubMenu = GetSubMenu(hMenu, 0);
			SetForegroundWindow(hwnd);
			TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
		}
		if (LOWORD(lParam)== WM_LBUTTONDBLCLK)
		{
			/*
			 *    TODO: 
			 *              以后会在这里增加一个设置对话框
			 *    这个对话框的作用主要是设置相应的快捷键
			 *    最好对这个对话框进行一定的美化
			 *    对颜色快捷键的颜色匹配进行设置!
			 *
			 *            --------  2021/08/14     
			 *           Author： 丁翔宇  
			 */
			
		}
		break;
	}
	case WM_LBUTTONDOWN:
	{
		if (wParam == MK_LBUTTON)
		{
			if (is_succeed)
			{
				if (old_bmp != NULL)
				{
					hdc = GetDC(hwnd);
					HDC hMemDC = CreateCompatibleDC(hdc);
					SelectObject(hMemDC, old_bmp);
					BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
					ReleaseDC(hwnd, hdc);
					DeleteObject(old_bmp);
				}
				is_succeed = false;
			}
			WORD x = LOWORD(lParam);
			WORD y = HIWORD(lParam);
			old_x = x;
			old_y = y;
			ltpoint.x = x;
			ltpoint.y = y;
		}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		WORD x = LOWORD(lParam);
		WORD y = HIWORD(lParam);
		HMENU menu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU1));
		HMENU pop_menu = GetSubMenu(menu, 0);
		if (is_checked)
		{
			CheckMenuItem(pop_menu, ID_HELP, MF_BYCOMMAND | MF_CHECKED);
		}
		else {
			CheckMenuItem(pop_menu, ID_HELP, MF_BYCOMMAND | MF_UNCHECKED);
		}
		TrackPopupMenu(pop_menu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, hwnd, NULL);
		DestroyMenu(menu);
		break;
	}
	case WM_COMMAND:
	{
		WORD id = LOWORD(wParam);
		if (id == ID_SHOW)
		{
			if (is_window_hidden)
			{
				ShowWindow(hwnd, SW_SHOWNORMAL);
				hdc = GetDC(hwnd);
				HDC hMemDC = CreateCompatibleDC(hdc);
				SelectObject(hMemDC, old_bmp);
				BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
				DeleteDC(hMemDC);
				ReleaseDC(hwnd, hdc);	
				is_window_hidden = false;
			}
		}
		if (id == ID_QUIT)
		{
			Shell_NotifyIcon(NIM_DELETE, &notify_i_data);
			PostQuitMessage(0);
		}
		if (HIWORD(wParam) == 1 && id == ID_SAVE && is_sounds_open)
		{
			PlaySound(MAKEINTRESOURCE(IDR_WAVE1), g_hInst, SND_ASYNC | SND_RESOURCE);
		}
		if (HIWORD(wParam) == 0 && id == ID_SAVE && is_sounds_open)
		{
			PlaySound(MAKEINTRESOURCE(IDR_WAVE2), g_hInst, SND_ASYNC | SND_RESOURCE);
		}
		if (id == ID_SAVE)
		{
			TCHAR szFilePath[MAX_PATH] = { 0 };
			PCTSTR filter_string = TEXT("位图(*.bmp)\0*.bmp\0关于成果的(*.txt)\0*.txt\0所有文件(*.*)\0*.*\0");
			OPENFILENAME ofi;
			ofi.lStructSize = sizeof(OPENFILENAME);
			ofi.hwndOwner = hwnd;
			ofi.hInstance = NULL;
			ofi.lpstrFilter = filter_string;
			ofi.lpstrCustomFilter = NULL;
			ofi.nMaxCustFilter = 0;
			ofi.nFilterIndex = 2;
			ofi.lpstrFile = szFilePath;
			ofi.nMaxFile = MAX_PATH;
			ofi.lpstrFileTitle = NULL;
			ofi.nMaxFileTitle = 0;
			ofi.lpstrInitialDir = NULL;
			ofi.lpstrTitle = TEXT("关于成果的...");
			ofi.Flags = 0;
			ofi.nFileOffset = 0;
			ofi.nFileExtension = 0;
			ofi.lpstrDefExt = TEXT("bmp");
			ofi.lCustData = 0;
			ofi.lpfnHook = NULL;
			ofi.lpTemplateName = NULL;
			ofi.FlagsEx = 0;
			hdc = GetDC(hwnd);
			old_bmp = GetBMP(hdc, width, height);
			ReleaseDC(hwnd, hdc);
			GetSaveFileName(&ofi);
			SaveFile(hwnd, szFilePath, ofi.nFileExtension);
		}
		if (id == ID_HELP)
		{
			hdc = GetDC(hwnd);
			RECT rect;
			rect.left = width / 2 - 200;
			rect.right = width / 2 + 200;
			rect.top = height / 2 - 400;
			rect.bottom = height / 2 - 170;
			if (!is_checked)
			{
				is_checked = true;
				int point_size = 15;
				HFONT hfont =
					CreateFont(-MulDiv(point_size, GetDeviceCaps(hdc, LOGPIXELSY), 72),
						0, 0, 0, 400, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"微软雅黑");
				HFONT hOldFont = (HFONT)SelectObject(hdc, hfont);
				HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
				SelectObject(hdc, hBrush);
				TCHAR content[] = TEXT("本软件提供以下按键:\n         q:笔     w:矩形     e:椭圆     r:橡皮\n         y:黄色     g:绿色     b:黑色\n         [:缩小字号     ]:增大字号\n         Ctrl+S:保存图片或文件\n注:     a:开启特效音    s:关闭特效音\n       Ctrl+Alt+Q:真退出    ESC:假退出\n       Ctrl+Alt+X:窗口还原    I:擦除屏幕");
				DrawText(hdc, content, -1, &rect, DT_TOP | DT_LEFT);
				ReleaseDC(hwnd, hdc);
				DeleteObject(hOldFont);
				DeleteObject(hfont);
				DeleteObject(hBrush);
			}
			else {
				HBRUSH hBrush = CreateSolidBrush(COLOR_TRANSPARENT);
				FillRect(hdc, &rect, hBrush);
				is_checked = false;
				DeleteObject(hBrush);
			}
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		if (wParam&MK_LBUTTON)
		{
			WORD x = LOWORD(lParam);
			WORD y = HIWORD(lParam);
			hdc = GetDC(hwnd);
			if (shape == SHAPE::PEN)
			{
				HPEN pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], pen_color);
				HPEN hOldPen = (HPEN)SelectObject(hdc, pen);
				MoveToEx(hdc, old_x, old_y, NULL);
				LineTo(hdc, x, y);
				old_x = x;
				old_y = y;
				ReleaseDC(hwnd, hdc);
				DeleteObject(hOldPen);
				DeleteObject(pen);
			}
			else if (shape == SHAPE::CIRCLE)
			{
				if (pen_widths[pen_widths_index]<14)
				{
					HPEN pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], COLOR_TRANSPARENT);
					HPEN hOldPen = (HPEN)SelectObject(hdc, pen);
					Arc(hdc, ltpoint.x, ltpoint.y, old_x, old_y, ltpoint.x, ltpoint.y / 2, ltpoint.x, ltpoint.y / 2);
					old_x = x;
					old_y = y;
					DeleteObject(hOldPen);
					pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], pen_color);
					hOldPen = (HPEN)SelectObject(hdc, pen);
					Arc(hdc, ltpoint.x, ltpoint.y, x, y, ltpoint.x, ltpoint.y / 2, ltpoint.x, ltpoint.y / 2);
					DeleteObject(hOldPen);
					DeleteObject(pen);
					ReleaseDC(NULL, hdc);
				}
				else {
					HDC hMemDC = CreateCompatibleDC(hdc);
					HPEN pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], COLOR_TRANSPARENT);
					HPEN hOldPen = (HPEN)SelectObject(hMemDC, pen);
					HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, width, height);
					SelectObject(hMemDC, hMemBmp);
					BitBlt(hMemDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
					Arc(hMemDC, ltpoint.x, ltpoint.y, old_x, old_y, ltpoint.x, ltpoint.y / 2, ltpoint.x, ltpoint.y / 2);
					old_x = x;
					old_y = y;
					DeleteObject(hOldPen);
					pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], pen_color);
					hOldPen = (HPEN)SelectObject(hMemDC, pen);
					Arc(hMemDC, ltpoint.x, ltpoint.y, x, y, ltpoint.x, ltpoint.y / 2, ltpoint.x, ltpoint.y / 2);
					BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
					DeleteObject(hOldPen);
					DeleteObject(pen);
					ReleaseDC(NULL, hdc);
					DeleteDC(hMemDC);
					DeleteObject(hMemBmp);
				}			
			}
			else if (shape == SHAPE::RECTANGLE)
			{
				POINT old_pt, same_as_lt_y, same_as_lt_x;
				old_pt.x = old_x;
				old_pt.y = old_y;
				same_as_lt_x.x = ltpoint.x;
				same_as_lt_x.y = old_pt.y;
				same_as_lt_y.x = old_pt.x;
				same_as_lt_y.y = ltpoint.y;
				POINT pt[5] = { ltpoint,same_as_lt_y,old_pt,same_as_lt_x,ltpoint };
				HPEN pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], COLOR_TRANSPARENT);
				HDC hMemDC = CreateCompatibleDC(hdc);
				HBITMAP hMemBmp = CreateCompatibleBitmap(hdc, width, height);
				SelectObject(hMemDC, hMemBmp);
				BitBlt(hMemDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
				HPEN hOldPen = (HPEN)SelectObject(hMemDC, pen);
				Polyline(hMemDC, pt, _countof(pt));
				old_x = x;
				old_y = y;
				DeleteObject(hOldPen);
				pen = CreatePen(PS_SOLID, pen_widths[pen_widths_index], pen_color);
				hOldPen = (HPEN)SelectObject(hMemDC, pen);
				POINT new_point{ x,y };
				same_as_lt_x.x = ltpoint.x;
				same_as_lt_x.y = old_y;
				same_as_lt_y.x = old_x;
				same_as_lt_y.y = ltpoint.y;
				POINT pt2[5] = { ltpoint,same_as_lt_y,new_point,same_as_lt_x,ltpoint };
				Polyline(hMemDC, pt2, _countof(pt2));
				BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
				DeleteObject(hOldPen);
				DeleteObject(pen);
				ReleaseDC(hwnd, hdc);
				DeleteDC(hMemDC);
				DeleteObject(hMemBmp);
			}
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		HDC hMemDC = CreateCompatibleDC(hdc);
		SelectObject(hMemDC, old_bmp);
		BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);
		DeleteDC(hMemDC);
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			if (old_bmp)
			{
				DeleteObject(old_bmp);
			}
			//DestroyWindow(hwnd);
			hdc = GetDC(hwnd);
			old_bmp = GetBMP(hdc, width, height);
			ReleaseDC(hwnd, hdc);
			ShowWindow(hwnd, SW_HIDE);
			is_window_hidden = true;
		}
		//if (wParam == 0xDD) 
		if(GetKeyState(0xDD)&0x8000)
		{
			if (++pen_widths_index >= _countof(pen_widths))
				pen_widths_index = _countof(pen_widths) - 1;
		}
		//if (wParam == 0xDB)
		if (GetKeyState(0xDB) & 0x8000)
		{
			if (--pen_widths_index < 0)
			{
				pen_widths_index = 0;
			}
		}
		//if (wParam == 0x59)  //Y
		if (GetKeyState(0x59) & 0x8000)
		{
			pen_color = RGB(255, 255, 0);
		}
		//if (wParam == 0x49)   //I
		if (GetKeyState(0x49) & 0x8000)
		{
			HDC hdc = GetDC(hwnd);
			HBRUSH hBrush = CreateSolidBrush(COLOR_TRANSPARENT);
			RECT rect;
			GetWindowRect(hwnd, &rect);
			if (is_checked)
			{
				RECT text_rect;
				text_rect.left = width / 2 - 200;
				text_rect.right = width / 2 + 200;
				text_rect.top = height / 2 - 400;
				text_rect.bottom = height / 2 - 170;
				HRGN big_rgn = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
				HRGN small_rgn = CreateRectRgn(text_rect.left, text_rect.top, text_rect.right, text_rect.bottom);
				CombineRgn(big_rgn, big_rgn, small_rgn, RGN_DIFF);
				FillRgn(hdc, big_rgn, hBrush);
			}
			else {
				FillRect(hdc, &rect, hBrush);
			}
			DeleteObject(hBrush);
			ReleaseDC(hwnd, hdc);

		}
		//if (wParam == 0x47) //G
		if (GetKeyState(0x47) & 0x8000)
		{
			pen_color = RGB(0, 255, 0);
		}
		//if (wParam == 0x42) //B
		if (GetKeyState(0x42) & 0x8000)
		{
			pen_color = RGB(0, 0, 0);
		}
		//if (wParam == 0x52) //R
		if (GetKeyState(0x52) & 0x8000)
		{
			is_erase = true;
			SetClassLong(hwnd, GCL_HCURSOR, NULL);
			HCURSOR hCursor =LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
			SetCursor(hCursor);
			ShowCursor(TRUE);
			if (!stack.empty() && pen_color != COLOR_TRANSPARENT)
			{
				stack.pop();
			}
			if (stack.empty())
			{
				stack.push(pen_color);
			}
			pen_color = COLOR_TRANSPARENT;
			shape = SHAPE::PEN;
		}
		//if (wParam == 0x51)  //Q
		if (GetKeyState(0x51) & 0x8000)
		{
			is_erase = false;
			SetClassLong(hwnd, GCL_HCURSOR,(LONG)LoadCursor(NULL, IDC_ARROW));
			shape = SHAPE::PEN;
			if (!stack.empty())
			{
				pen_color = stack.top();
				stack.pop();
			}
		}
		//if (wParam == 0x57) //W
		if (GetKeyState(0x57) & 0x8000)
		{
			is_erase = false;
			SetClassLong(hwnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
			shape = SHAPE::RECTANGLE;
			if (!stack.empty())
			{
				pen_color = stack.top();
				stack.pop();
			}
		}
	   //if (wParam == 0x45) //E
		if (GetKeyState(0x45) & 0x8000)
		{
			is_erase = false;
			SetClassLong(hwnd, GCL_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
			shape = SHAPE::CIRCLE;
			if (!stack.empty())
			{
				pen_color = stack.top();
				stack.pop();
			}
		}
		//if (wParam == 0x41)//A
		if (GetKeyState(0x41) & 0x8000)
		{
			is_sounds_open = true;
		}
		//if (wParam == 0x53) //S
		if (GetKeyState(0x53) & 0x8000)
		{
			is_sounds_open = false;
		}
		break;
	}
	case WM_SETCURSOR:
	{
		if (is_erase)
		{
			HCURSOR hCursor = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
			SetCursor(hCursor);
		}
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
