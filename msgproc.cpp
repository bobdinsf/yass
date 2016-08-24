#include "stdafx.h"
#include "yass.h"
#include <WindowsX.h>
#include <complex>
#include "mset.h"
#include "img.h"

using namespace std;
extern HINSTANCE hInst;								// current instance
extern INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//h
CMSet* g_pmz;
static bool down = false;
static bool up = false;
static RECT rbrBand;
static RECT panRect;
static RECT clientR;
static UINT_PTR nRefreshTimerID = 0xbd;
static const UINT nRefreshMsecs = 50;
enum timers
{
	eDblClkTimer,
};
void doMouseBusiness(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void doZoomOut();
void doZoomIn();
void startRefresh(HWND hWnd)
{
	if (nRefreshTimerID != SetTimer(hWnd, nRefreshTimerID, nRefreshMsecs, NULL))
	{
		dprintf(L"Failed to start timer");
	}
	dprintf(L"Started timer %d", nRefreshTimerID);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int dbgCount = 0;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{

	case WM_TIMER:
		dprintf(L"Timer message tick = %d, n=%d",GetTickCount(), g_pmz->getNComputeThreads());
		if (g_pmz->getNComputeThreads() > 0)
		{
			startRefresh(hWnd);
		}
		else
		{
			dprintf(L"KillTimer %d", nRefreshTimerID);
			if (!KillTimer(hWnd, nRefreshTimerID))
			{
				dprintf(L"KillTimer failed:  %d", GetLastError());
			}
			nRefreshTimerID = 0;
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_ZOOM_OUT:
			doZoomOut();
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_ZOOM_IN:
			doZoomIn();
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_ZOOM_RESET:
			g_pmz->set(-2.0,-2.0,2.0,2.0);
			g_pmz->compute();
			startRefresh(hWnd);
			g_pmz->SetThreshold(CMSet::nInitialThreshold);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_THRESHOLD_HALF:
			g_pmz->HalfThreshold();
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_THRESHOLD_DOUBLE:
			g_pmz->DoubleThreshold();
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case ID_THRESHOLD_RESET:
			g_pmz->SetThreshold(g_pmz->nInitialThreshold);
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_KEYDOWN:
		{
			bool bNeedRefresh = true;
			switch (wParam)
			{
				case VK_LEFT:
					g_pmz->prev();
					break;
				case VK_RIGHT:
					g_pmz->next();
					break;
				case VK_UP:
					g_pmz->DoubleThreshold();
					break;
				case VK_DOWN:
					g_pmz->HalfThreshold();
					break;
				default:
					bNeedRefresh = false;
					break;
			}
			if (bNeedRefresh)
			{
				g_pmz->compute();
				startRefresh(hWnd);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONDBLCLK:
		doMouseBusiness(hWnd, message, wParam, lParam);
		break;
	case WM_SIZE:
		{
			if (g_pmz->getNComputeThreads() == 0)
			{
				g_pmz->resize(LOWORD(lParam),HIWORD(lParam));
			}
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
    case WM_CREATE:
		{
			LPCREATESTRUCT pcs = (CREATESTRUCT*)lParam;
			g_pmz = new CMSet;
			g_pmz->resize(pcs->cx,pcs->cy);
			g_pmz->SetThreshold(g_pmz->nInitialThreshold);
			g_pmz->SetWindowHandle(hWnd);
			g_pmz->compute();
			startRefresh(hWnd);
		}
	    break;
	case WM_PAINT:
		{
			dprintf(L"WM_PAINT:  %d",++dbgCount);
			hdc = BeginPaint(hWnd, &ps);
			HDC hdcsrc = g_pmz->GetBitmap(hdc);
			BitBlt(hdc,0,0,g_pmz->GetWidth(),g_pmz->GetLength(),hdcsrc,0,0,SRCCOPY);

			//TCHAR s[200];
			//_stprintf(s,L"%d,%d,%d,%d:x0=%f,y0=%f,x1=%f,y1=%f",rbrBand.left,rbrBand.top,rbrBand.right,rbrBand.bottom,g_pmz->GetX0(),g_pmz->GetY0(),g_pmz->GetX1(),g_pmz->GetY1());
			//RECT clientR;
			//GetClientRect(hWnd,&clientR);
			//DrawText(hdc,s,-1, &clientR,DT_LEFT);
			EndPaint(hWnd, &ps);
		}
        break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



void NormRect(RECT& rect)
{
	if (rect.left > rect.right)
		swap(rect.left,rect.right);
	if (rect.top > rect.bottom)
		swap(rect.top,rect.bottom);
}
void DrawRect(HDC hdc,RECT rect)
{
	Rectangle(hdc,rect.left,rect.top,rect.right,rect.bottom);
}

void doMouseBusiness(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
	case WM_MOUSEMOVE:
		if (down)
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			// Need to redraw the rubber band rectancle. 
			HDC hdc = GetDC(hWnd);
			SetROP2(hdc,R2_XORPEN);

			// erase the last rubber band.
			DrawRect(hdc,rbrBand);
				
			rbrBand.right = x;				
			rbrBand.bottom = y;

			// draw the new rubber band.
			DrawRect(hdc,rbrBand);

			//InvalidateRect(hWnd,&rbrBand,FALSE);
		}
		if (up)
		{
		}
		break;
	case WM_LBUTTONDBLCLK:
		// On double click zoom 200% from location of double click
		{
			RECT zrect;
			int x = GET_X_LPARAM(lParam); 
			int y = GET_Y_LPARAM(lParam); 
			int w = g_pmz->GetWidth();
			int l = g_pmz->GetLength();
			zrect.left = x - (w / 2);
			if (zrect.left < 0) zrect.left = 0;
			zrect.right = y + (w / 2);//
			if (zrect.right >= w) zrect.right = w - 1;
			zrect.top = y - (l / 2);
			if (zrect.top < 0) zrect.top = 0;
			zrect.bottom = y + (l / 2);
			if (zrect.bottom >= l) zrect.right = l - 1;
			g_pmz->zoom(zrect);
			g_pmz->compute();
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_LBUTTONDOWN:
		{
			down = true;
			HDC hdc = GetDC(hWnd);
			SetROP2(hdc,R2_XORPEN);
			rbrBand.right = rbrBand.left = LOWORD(lParam);
			rbrBand.top = rbrBand.bottom = HIWORD(lParam);
			DrawRect(hdc,rbrBand);
		}
		break;
	case WM_LBUTTONUP:
		{
			down = false;
			// Need to redraw the rubber band rectancle. 
			HDC hdc = GetDC(hWnd);
			SetROP2(hdc,R2_XORPEN);

			// erase the last rubber band.
			DrawRect(hdc,rbrBand);

			if (rbrBand.left > rbrBand.right) 
				swap(rbrBand.left,rbrBand.right);
			if (rbrBand.top > rbrBand.bottom) 
				swap(rbrBand.top,rbrBand.bottom);

			// zoom in
			if (rbrBand.bottom - rbrBand.top > 10)
			{
				g_pmz->zoom(rbrBand);
				g_pmz->compute();
				startRefresh(hWnd);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;
	case WM_RBUTTONDOWN:
		{
			panRect.left = LOWORD(lParam);
			panRect.top = HIWORD(lParam);
		}
		break;
	//case WM_RBUTTONUP:
	//	{
	//		panRect.right = LOWORD(lParam);
	//		panRect.bottom = HIWORD(lParam);
	//	}
	//	break;
	case WM_RBUTTONUP:
		{
			doZoomOut();
			g_pmz->compute();
			startRefresh(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	default:
		break;
	}

}

void doZoomOut()
{
	double x0 = g_pmz->GetX0();
	double x1 = g_pmz->GetX1();
	double w = x1 - x0;
	double y0 = g_pmz->GetY0();
	double y1 = g_pmz->GetY1();
	double h = y1 - y0;
	g_pmz->set(x0 - w/2,y0 - h/2,x1 + w/2,y1 + h/2);
}

void doZoomIn()
{
	int w = g_pmz->GetWidth();
	int h = g_pmz->GetLength();
	RECT zrect;
	zrect.left = w / 4;
	zrect.right = 3 * w / 4;
	zrect.top = h / 4;
	zrect.bottom = 3 * h / 4;
	g_pmz->zoom(zrect);
}