#ifndef WINDOW_PLATEFORM
#define WINDOW_PLATEFORM

#include <Windows.h>
#include <stdint.h>

#include <gl/GL.h>
#include <gl/glext.h>
#include <gl/wglext.h>

extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;


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
