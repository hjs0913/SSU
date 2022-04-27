// LabProject08-5.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "test1.h"
#include "GameFramework.h"
#include "Bitmap.h"
#include "UILayer.h" 
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console") //콘솔 띄우자 
#define MAX_LOADSTRING 100

HINSTANCE						ghAppInstance;
TCHAR							szTitle[MAX_LOADSTRING];
TCHAR							szWindowClass[MAX_LOADSTRING];

CGameFramework					gGameFramework;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);


#pragma comment(lib, "WindowsCodecs.lib")
#pragma comment(lib, "D2D1.lib")



int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	setlocale(LC_ALL, "");

	// connect network
	netInit();
	thread hThread{ worker };

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	::LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	::LoadString(hInstance, IDC_LABPROJECT085, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow)) return(FALSE);

	hAccelTable = ::LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABPROJECT085));

	bool change_dungeon = false;

	while (1)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			if (!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		else
		{
			if (change_dungeon != InDungeon) {
				change_dungeon = InDungeon;
				if (change_dungeon) {
					gGameFramework.Release_OpenWorld_Object();
					gGameFramework.Create_InDungeon_Object();
					send_raid_rander_ok_packet();
				//	send_partner_rander_ok_packet();
				}
				else {
					gGameFramework.Release_InDungeon_Object();
					gGameFramework.Create_OpenWorld_Object();
				}
			}

			gGameFramework.FrameAdvance();
		}
	}
	gGameFramework.OnDestroy();

	return((int)msg.wParam);
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABPROJECT085));
	wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;//MAKEINTRESOURCE(IDC_LABPROJECT085);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = ::LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	ghAppInstance = hInstance;

	RECT rc ={ 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
	AdjustWindowRect(&rc, dwStyle, FALSE);
	HWND hMainWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hMainWnd) return(FALSE);

	gGameFramework.OnCreate(hInstance, hMainWnd);

	::ShowWindow(hMainWnd, nCmdShow);
	::UpdateWindow(hMainWnd);

#ifdef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	gGameFramework.ChangeSwapChainState();
#endif

	return(TRUE);
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TRACKMOUSEEVENT tme;

	static HIMC imeID = nullptr;
	switch (message)
	{
	case WM_CREATE:
		break;
	case WM_MOUSELEAVE:
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.hwndTrack = hWnd;
		tme.dwFlags = TME_HOVER;
		tme.dwHoverTime = 1;
		TrackMouseEvent(&tme);
		Mouse_On = false;
		break;
	case WM_MOUSEHOVER:
	case WM_SIZE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
	case WM_KEYDOWN:
	case WM_KEYUP:
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.hwndTrack = hWnd;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 1;
		TrackMouseEvent(&tme);
		Mouse_On = true;
		gGameFramework.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
		break;
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBox(ghAppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			SAFE_RELEASE(m_pD2DFactory);
			SAFE_RELEASE(m_pWICFactory);
			SAFE_RELEASE(m_pRenderTarget);
			SAFE_RELEASE(m_pGridPatternBitmapBrush);
			SAFE_RELEASE(m_pBitmap);

			::DestroyWindow(hWnd);
			break;
		default:
			return(::DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;
	case WM_PAINT:

		hdc = ::BeginPaint(hWnd, &ps);
	
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		ImmDestroyContext(imeID);
		::PostQuitMessage(0);
		break;

	case WM_ACTIVATE:
		if (imeID == nullptr)
		{
			imeID = ImmCreateContext();
			ImmAssociateContext(hWnd, imeID);
		}
	case WM_IME_STARTCOMPOSITION:
		break;
	case WM_IME_CHAR:
	case WM_CHAR: {
		if (PartyInviteUI_ON) {
			if ((wchar_t)wParam == '\b') {
				if (Invite_Str.size() > 0) {
					Invite_Str.pop_back();
				}
			}
			else if ((wchar_t)wParam == '\r') break;
			else {
				if (Invite_Str.size() <= 20)
				{
					Invite_Str.push_back((wchar_t)wParam);
				}
			}
		}

		if (Chatting_On) {
			if ((wchar_t)wParam == '\b') {
				if (Chatting_Str.size() > 0) {
					Chatting_Str.pop_back();
				}
			}
			else {
				if (Chatting_Str.size() <= 20)
				{
					Chatting_Str.push_back((wchar_t)wParam);
				}
			}
		}

		break;
	}

	default:
		return(::DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return((INT_PTR)TRUE);
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return((INT_PTR)TRUE);
		}
		break;
	}
	return((INT_PTR)FALSE);
}
