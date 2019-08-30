#include <stdio.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/glext.h>
#include <gl/wglext.h>

LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
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
ATOM register_class(HINSTANCE h_instance)
{
	WNDCLASSA wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	//wcex.cbSize = sizeof(wcex);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc = window_procedure;
	wcex.hInstance = h_instance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszClassName = "Core";

	return RegisterClass(&wcex);
}

void show_message(LPCSTR message)
{
	MessageBox(0, message, "Window", MB_ICONERROR);
}

#define LOAD_PFN(Type, pfn) do {\
pfn = (Type)wglGetProcAddress(#pfn);\
if (pfn == NULL)\
{\
	show_message(#pfn "() failed.");\
	return 1;\
}\
} while(0)


int APIENTRY wWinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPWSTR lp_cmd_line, int n_cmd_show) 
{
	LPSTR window_class = MAKEINTATOM(register_class(h_instance));

	HWND fake_wnd = CreateWindow(
		"Core", "Fake Window",
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0,
		1, 1,
		NULL, NULL,
		h_instance, NULL
	);

	HDC fake_dc = GetDC(fake_wnd);
	if (fake_dc == 0)
	{
		show_message("GetDC() failed");
		return 1;
	}

	PIXELFORMATDESCRIPTOR fake_pfd;
	ZeroMemory(&fake_pfd, sizeof(fake_pfd));
	fake_pfd.nSize = sizeof(fake_pfd);
	fake_pfd.nVersion = 1;
	fake_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	fake_pfd.iPixelType = PFD_TYPE_RGBA;
	fake_pfd.cColorBits = 32;
	fake_pfd.cAlphaBits = 8;
	fake_pfd.cDepthBits = 24;

	int fake_pfd_id = ChoosePixelFormat(fake_dc, &fake_pfd);
	if (fake_pfd_id == 0)
	{
		show_message("ChoosePixelFormat() failed");
		return 1;
	}

	if (SetPixelFormat(fake_dc, fake_pfd_id, &fake_pfd) == 0)
	{
		show_message("SetPixelFormat() failed");
		return 1;
	}

	HGLRC fake_rc = wglCreateContext(fake_dc);

	if (fake_rc == 0)
	{
		show_message("wglCreateContext() failed");
		return 1;
	}

	if (wglMakeCurrent(fake_dc, fake_rc) == 0)
	{
		show_message("wglMakeCurrent() failed");
		return 1;
	}

	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
	LOAD_PFN(PFNWGLCHOOSEPIXELFORMATARBPROC, wglChoosePixelFormatARB);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	LOAD_PFN(PFNWGLCREATECONTEXTATTRIBSARBPROC, wglCreateContextAttribsARB);

	// create the real window
	HWND wnd = CreateWindow(
		"Core", "OpenGL Window",
		 WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0, 0,
		800, 100,
		NULL, NULL,
		h_instance, NULL
	);
	
	HDC dc = GetDC(wnd);

	const int pixel_attribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 4,
		0
	};

	int pixel_format_id; 
	UINT num_formats;
	int status = wglChoosePixelFormatARB(dc, pixel_attribs, NULL, 1, &pixel_format_id, &num_formats);

	if (status == 0 || num_formats == 0)
	{
		show_message("wglChoosePixelFormatARB() failed");
		return 1;
	}

	PIXELFORMATDESCRIPTOR pfd;
	DescribePixelFormat(dc, pixel_format_id, sizeof(pfd), &pfd);
	SetPixelFormat(dc, pixel_format_id, &pfd);

	const int major_min = 4;
	const int minor_min = 5;
	int context_attribs[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, major_min,
		WGL_CONTEXT_MINOR_VERSION_ARB, minor_min,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	HGLRC rc = wglCreateContextAttribsARB(dc, 0, context_attribs);
	if (rc == NULL)
	{
		show_message("wglCreateContextAttribsARB() failed");
		return 1;
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(fake_rc);
	ReleaseDC(fake_wnd, fake_dc);
	DestroyWindow(fake_wnd);
	if (!wglMakeCurrent(dc, rc))
	{
		show_message("wglMakeCurrent() failed");
		return 1;
	}


	SetWindowText(wnd, (LPCSTR)glGetString(GL_VERSION));
	ShowWindow(wnd, n_cmd_show);


	MSG msg;
	int active = 1;
	while (active) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				active = 0;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		glClearColor(0.129f, 0.586f, 0.949f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SwapBuffers(dc);
	}


	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(rc);
	ReleaseDC(wnd, dc);
	DestroyWindow(wnd);

	return msg.wParam;
}