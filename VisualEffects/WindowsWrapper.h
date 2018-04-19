#pragma once

// A window class that creates a window
class WindowsWrapper
{
public:
	WindowsWrapper(int width, int height, TCHAR *windowName, TCHAR *className); //constructor
	~WindowsWrapper();	//destructor
	void Tick(); //tick is called every frame
	bool QuitRequested() const; //return true if quit requested	
	LRESULT LocalWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND GetHwnd() { return hwnd; }

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); // I encode the hwnd on the window

private:
	WindowsWrapper(); //hide default constructor

	HWND hwnd; //handle variable
	bool quitRequested_;	

};
