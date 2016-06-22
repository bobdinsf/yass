// yass.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "yass.h"
#include <WindowsX.h>
#include <complex>
#include "img.h"
using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HBITMAP g_hbmBall = NULL;
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
//complex c2double ( 1.0, 1.0 );
extern Cmset* g_pmz;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	Img img;
	Img* p = &img;
	//OutputDebugString(__FILE__ L" " __DATE__ L"," __TIME__);
	
	wchar_t f[255];
	wchar_t d[255];
	wchar_t t[255];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, __FILE__, -1, f, sizeof(f));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, __DATE__, -1, d, sizeof(d));
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, __TIME__, -1, t, sizeof(t));
	dprintf(L"%s compiled %s at %s.", f, d, t);
#endif

	if (!unitTestsForRealRect())
	{
		MessageBox(NULL,L"unit test failed",L"debug",MB_ICONERROR);
		return -1;
	}
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_YASS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YASS));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YASS));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_YASS);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, Cmset::nInitWidth, Cmset::nInitLength, NULL, NULL, hInstance, NULL);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}
#if 0
void ShowAnImage(HWND hWnd)
{
	RECT rcClient;
	HDC hdcScreen = GetDC(NULL);
	HDC hdcWindow = GetDC(hWnd);
	HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

	//This is the best stretch mode
	SetStretchBltMode(hdcWindow,HALFTONE);

	GetClientRect(hWnd, &rcClient);
	HBITMAP memBM = CreateCompatibleBitmap(hdcMemDC,rcClient.right,rcClient.top);
	HGDIOBJ oldObject = SelectObject(hdcMemDC,memBM);
	
	// TODO: Add any drawing code here...
		RECT rect;
		rect.bottom = 0;
		rect.left = 0;
		rect.top = 100;
		rect.right = 100;
		HBRUSH hbrush = CreateSolidBrush(RGB(0,0,255));
		FillRect(hdcMemDC, &rect, hbrush);
		MoveToEx(hdcMemDC,100,100,(LPPOINT) NULL);
		LineTo(hdcMemDC,0,0);

	if (!StretchBlt(hdcWindow,
					0,0,
					rcClient.right, rcClient.bottom,
					hdcMemDC,
					0,0,
					256, //GetSystemMetrics (SM_CXSCREEN),
					256, // GetSystemMetrics (SM_CYSCREEN),
					SRCCOPY))
	{
		MessageBox(hWnd, L"StretchBlt2 has failed",L"oops",MB_ICONERROR);
	}
	
}
#endif


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


	/*if(!StretchBlt(hdcWindow, 
				0,0, 
				rcClient.right, rcClient.bottom, 
				hdcScreen, 
				0,0,
				GetSystemMetrics (SM_CXSCREEN),
				GetSystemMetrics (SM_CYSCREEN),
				SRCCOPY))
	{
		MessageBox(hWnd, L"StretchBlt has failed",L"Failed", MB_OK);
	}
	*/

bool unitTestsForRealRect()
{
	RealRect mr(500,500,-5,-5,5,5);
	mr.zoom(200,300,200,300);
	std::pair<double,double> pt;
	if  ( mr.getX0() != -1 || mr.getY0() != -1)
	{
		MessageBox(NULL,L"wrong",L"debug",MB_OK);
	}
	mr.zoom(0,250,0,250);
	if ( mr.getX0() != -1 || mr.getY0() != 0)
	{
		MessageBox(NULL,L"wrong",L"debug",MB_OK);
	}
	
	return true;
}