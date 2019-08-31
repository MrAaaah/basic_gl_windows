#ifndef WINDOW_PLATEFORM
#define WINDOW_PLATEFORM

#include <Windows.h>
#include <stdint.h>

typedef struct WindowData
{
	// need to be filled by user
	HINSTANCE h_instance; 
	WNDPROC window_proc;

	// filled by create_window
	HWND h_wnd;
	HDC dc;
	HGLRC rc;
	HGLRC rc2;
} WindowData;

void show_message(LPCSTR message);

int create_window(WindowData * window_data,
	int32_t x, int32_t y, int32_t w, int32_t h);

#endif
