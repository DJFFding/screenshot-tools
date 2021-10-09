#include"about_window.h"

bool is_succeed = false;
HANDLE hEvent = CreateEvent(NULL, FALSE,FALSE,NULL);
BOOL SaveFile(HWND hwnd, PCTSTR szFileName, DWORD fileExtension)
{
	int len = _tcslen(szFileName);
	int extension_len = len - fileExtension;
	PTSTR szExtensionName = (PTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, extension_len * 2);
	for (size_t i = 0; i < extension_len; i++)
	{
		szExtensionName[i] = szFileName[fileExtension + i];
	}
	if (_tcscmp(szExtensionName, TEXT("bmp")) == 0)
	{
		RECT win_rect;
		HDC hdc = GetDC(NULL);
		int width= GetDeviceCaps(hdc,DESKTOPHORZRES);
		int height= GetDeviceCaps(hdc, DESKTOPVERTRES);
		HBITMAP bmp = GetBMP(hdc, width, height);
		LPVOID bmp_file = NULL;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		__try {
			BITMAP bimp = { 0 };
			GetObject(bmp, sizeof(bimp), &bimp);
			DWORD bmp_file_size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + bimp.bmWidthBytes*bimp.bmHeight;
			bmp_file = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bmp_file_size);
			if (bmp_file == NULL)
			{
				return 0;
			}
			PBITMAPFILEHEADER bmp_file_header = (PBITMAPFILEHEADER)bmp_file;
			PBITMAPINFO bmp_file_info = (PBITMAPINFO)((INT_PTR)bmp_file_header + sizeof(BITMAPFILEHEADER));
			PBITMAPINFOHEADER bmp_file_info_header = (PBITMAPINFOHEADER)bmp_file_info;
			bmp_file_header->bfType = 'MB';
			bmp_file_header->bfSize = bmp_file_size;
			bmp_file_header->bfReserved1 = 0;
			bmp_file_header->bfReserved2 = 0;
			bmp_file_header->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
			bmp_file_info_header->biSize = sizeof(BITMAPINFOHEADER);
			bmp_file_info_header->biWidth = bimp.bmWidth;
			bmp_file_info_header->biHeight = bimp.bmHeight;
			bmp_file_info_header->biPlanes = 1;
			bmp_file_info_header->biBitCount = bimp.bmBitsPixel;
			bmp_file_info_header->biCompression = BI_RGB;
			bmp_file_info_header->biSizeImage = 0;
			bmp_file_info_header->biXPelsPerMeter = GetDeviceCaps(hdc, HORZSIZE) / 1000;
			bmp_file_info_header->biYPelsPerMeter = GetDeviceCaps(hdc, VERTSIZE) / 1000;
			bmp_file_info_header->biClrUsed = 0;
			bmp_file_info_header->biClrImportant = 0;
			LPVOID bmp_file_content = (LPVOID)((INT_PTR)bmp_file + bmp_file_header->bfOffBits);
			GetDIBits(hdc, bmp, 0, bimp.bmHeight, bmp_file_content, bmp_file_info, DIB_RGB_COLORS);
			hFile = CreateFile(szFileName, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				return 0;
			}
			WriteFile(hFile, bmp_file, bmp_file_size, NULL, NULL);
		}
		__finally {
			if (hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);
			}
			if (bmp_file != NULL)
			{
				HeapFree(GetProcessHeap(), 0, bmp_file);
			}
			DeleteObject(bmp);
			ReleaseDC(NULL, hdc);
		}
		SetEvent(hEvent);
		is_succeed = true;
	}
	else {
		HANDLE hFile = CreateFile(szFileName, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		PTP_IO pio = CreateThreadpoolIo(hFile, IoCompletionCallback, NULL, NULL);
		OVERLAPPED over = { 0 };
		CHAR szText[] = "我爱你，成果!\r\n    这是属于成果的...\r\n                    -------翔宇的祝福...";
		StartThreadpoolIo(pio);
		WriteFile(hFile, szText, sizeof(szText), NULL, &over);
		WaitForThreadpoolIoCallbacks(pio, FALSE);
		CloseHandle(hFile);
		CloseThreadpoolIo(pio);
	}
	HeapFree(GetProcessHeap(), 0, szExtensionName);
	return TRUE;
}

VOID CALLBACK IoCompletionCallback(
	PTP_CALLBACK_INSTANCE Instance,
	PVOID                 Context,
	PVOID                 Overlapped,
	ULONG                 IoResult,
	ULONG_PTR             NumberOfBytesTransferred,
	PTP_IO                Io
)
{
	OVERLAPPED over = (*(LPOVERLAPPED)Overlapped);
	WaitForSingleObject(over.hEvent, INFINITE);
	if (IoResult == NO_ERROR)
	{
		SetEvent(hEvent);
		is_succeed = true;
	}
}

HBITMAP GetBMP(HDC hdc,int width,int height)
{
	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(hdc, width, height);
	SelectObject(hMemDC, hMemBitmap);
	BitBlt(hMemDC, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	DeleteDC(hMemDC);
	return hMemBitmap;
}