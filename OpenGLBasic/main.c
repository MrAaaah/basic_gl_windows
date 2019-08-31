#include <stdio.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/glext.h>
#include <gl/wglext.h>
#include <math.h>

#include "Window.h"

LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
	case WM_MOVING:
	{
		return TRUE;
	}
	case WM_KEYDOWN:
		if (w_param == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, w_param, l_param);
	}

	return 0;
}

RECT monitors_rect[3];
size_t monitors_count = 0;

BOOL CALLBACK MonitorEnumProc( __in  HMONITOR hMonitor, __in  HDC hdcMonitor, __in  LPRECT lprcMonitor, __in  LPARAM dwData )
{
	monitors_rect[monitors_count++] = *lprcMonitor;

    return TRUE;
}

typedef struct RenderingThreadData
{
	WindowData * window;

	HANDLE quit_mutex;
	BOOL run;
} RenderingThreadData;

DWORD WINAPI RenderThread(void* params) {
  // Do stuff.  This will be the first function called on the new thread.
  // When this function returns, the thread goes away.  See MSDN for more details.
	RenderingThreadData * data = params;
	if (!wglMakeCurrent(data->window->dc, data->window->rc))
	{
		show_message("wglMakeCurrent() failed");
		return 1;
	}
	BOOL active = TRUE;
	while (active)
	{
		RECT rect;
		if (GetWindowRect(data->window->h_wnd, &rect))
			glClearColor(rect.top / 3000.0, rect.left / 3000.0, 0, 1.0f);
		else
			glClearColor(0, 0, 0, 1);

		glClear(GL_COLOR_BUFFER_BIT);

		SwapBuffers(data->window->dc);

		DWORD waitResult = WaitForSingleObject(data->quit_mutex, INFINITE);
		active = data->run;
		ReleaseMutex(data->quit_mutex);
	}
  return 0;
}

int APIENTRY wWinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPWSTR lp_cmd_line, int n_cmd_show) 
{
	// compute the window size / position
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	size_t m = 0;
	float zoom = 0.5;

	float scr_width = (monitors_rect[m].right - monitors_rect[m].left);
	float scr_height = (monitors_rect[m].bottom - monitors_rect[m].top);

	float width = scr_width * zoom;
	float height = scr_height * zoom;

	float top = monitors_rect[m].top + (scr_width - width) * 0.5;
	float left = monitors_rect[m].left + (scr_height - height) * 0.5;

	// create the real window
	WindowData window;
	window.h_instance = h_instance;
	window.window_proc = window_procedure;

	int res = create_window(&window, top, left, width, height);

	if (res)
	{
		show_message("create_window() failed");
		return 1;
	}
	if (!wglMakeCurrent(window.dc, window.rc2))
	{
		show_message("wglMakeCurrent() failed");
		return 1;
	}

	SetWindowText(window.h_wnd, (LPCSTR)glGetString(GL_VERSION));
	ShowWindow(window.h_wnd, n_cmd_show);

	RenderingThreadData data;
	data.window = &window;
	data.quit_mutex = CreateMutex(NULL, FALSE, NULL);
	data.run = TRUE;

	HANDLE thread = CreateThread(NULL, 0, RenderThread, &data, 0, NULL);

	MSG msg;
	int active = 1;
	while (active) 
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			if (msg.message == WM_QUIT) 
			{
				active = 0;
			}
			DispatchMessage(&msg);
		}

		Sleep(10);
	}


	DWORD waitResult = WaitForSingleObject(data.quit_mutex, INFINITE);
	data.run = FALSE;
	ReleaseMutex(data.quit_mutex);

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
	CloseHandle(data.quit_mutex);


	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(window.rc);
	ReleaseDC(window.h_wnd, window.dc);
	DestroyWindow(window.h_wnd);

	return 0;// msg.wParam;
}