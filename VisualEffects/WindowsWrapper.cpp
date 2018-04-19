#include "stdafx.h"
#include <windowsx.h>
#include "WindowsWrapper.h"

//a very simple window class

// constructor for the Window wrapper
WindowsWrapper::WindowsWrapper(int width, int height, TCHAR *windowName, TCHAR *className)
{
	quitRequested_ = false;

	HINSTANCE hInstance = GetModuleHandle(NULL);

	// register a windows class
	WNDCLASSEX wcex;
	if (!GetClassInfoEx(hInstance, className, &wcex))
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(WindowsWrapper*);
		wcex.hInstance = hInstance;
		wcex.hIcon = 0; 
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = windowName;
		wcex.lpszClassName = className;
		wcex.hIconSm = 0; 
		ATOM wClass = RegisterClassEx(&wcex);
	}

	// create a window of that type
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	RECT wr = { 0, 0, width, height };
	AdjustWindowRect(&wr, dwStyle, FALSE);    // adjust the size
	hwnd = CreateWindowEx(0, className, windowName, dwStyle, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hInstance, NULL);

	// store a pointer to this on the GDI window
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
}
//destructor
WindowsWrapper::~WindowsWrapper()
{
	DestroyWindow(hwnd);
}

LRESULT CALLBACK WindowsWrapper::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowsWrapper *wrapper = (WindowsWrapper*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (wrapper) // we'll get messages during window construction before the windowlongptr is set
		return wrapper->LocalWndProc(hWnd, message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}

//the event handler for this window
LRESULT WindowsWrapper::LocalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_CLOSE:
		quitRequested_ = true;
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//tick is called every frame
void WindowsWrapper::Tick()
{
	MSG msg;

	// Main message loop for our window
	while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool WindowsWrapper::QuitRequested() const
{
	return quitRequested_;
}
