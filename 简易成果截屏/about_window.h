#pragma once
#include<Windows.h>
#include<tchar.h>
LRESULT CALLBACK MyWindowProc(
	HWND   hwnd,
	 UINT   uMsg,
	WPARAM wParam,
	LPARAM lParam
);
VOID CALLBACK IoCompletionCallback(
        PTP_CALLBACK_INSTANCE Instance,
	    PVOID                 Context,
	    PVOID                 Overlapped,
	    ULONG                 IoResult,
	    ULONG_PTR             NumberOfBytesTransferred,
	    PTP_IO                Io
);
BOOL SaveFile(HWND hwnd,PCTSTR szFileName, DWORD fileExtension);
DWORD WINAPI DoWFile(LPVOID lpParam);
HBITMAP GetBMP(HDC hdc, int width, int height);
extern HANDLE hEvent;
extern bool is_succeed;