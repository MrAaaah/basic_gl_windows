#include <stdio.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/glext.h>
#include <gl/wglext.h>
#include <math.h>

#include "Window.h"

float time = 0.2;

LRESULT CALLBACK window_procedure(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
	switch (message)
	{
	case WM_MOVING:
	{
		return TRUE;
	}
	case WM_KEYDOWN:
		if (w_param == VK_LEFT)
		{
			time -= 0.01;
		}
		else if (w_param == VK_RIGHT)
		{
			time += 0.01;
		}
		else if (w_param == VK_ESCAPE)
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

uint32_t program;
uint32_t vao;
uint32_t fullscreen_program;
uint32_t fullscreen_vao;
uint32_t time_uniform;

size_t read_file(const char * path, char ** buffer)
{
	FILE * fh;
	fopen_s(&fh, path, "rb");
	fseek(fh, 0, SEEK_END);
	long length = ftell(fh);
	fseek(fh, 0, SEEK_SET);
	char * local_buffer = malloc(length + 1);
	if (local_buffer)
	{
		fread(local_buffer, 1, length, fh);
	}
	local_buffer[length] = '\0';

	fclose(fh);

	*buffer = local_buffer;
	return length;
}

uint32_t compile_shader(GLuint shader_type, char * content)
{
	int success;
	char info_log[512];

	// vertex Shader
	uint32_t shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, content, NULL);
	glCompileShader(shader);
	// print compile errors if any
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, info_log);
		show_message(info_log);
	};

	return shader;
}

typedef struct ShaderStage
{
	GLuint type;
	const char * filepath;
} ShaderStage;

uint32_t new_program(ShaderStage * shader_stages, size_t stages_count)
{
	GLuint * shaders = malloc(sizeof(GLuint) * stages_count);
	for (size_t i = 0; i < stages_count; i++)
	{
		char * content;
		read_file(shader_stages[i].filepath, &content);

		shaders[i] = compile_shader(shader_stages[i].type, &content);
		free(content);
	}

	GLuint program = glCreateProgram();
	for (size_t i = 0; i < stages_count; i++)
	{
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);
	// print linking errors if any

	int success;
	char info_log[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, info_log);
		show_message(info_log);
	}

	// delete the shaders as they're linked into our program now and no longer necessery
	for (size_t i = 0; i < stages_count; i++)
	{
		glDeleteShader(shaders[i]);
	}
	free(shaders);

	return program;
}

void renderer_init()
{
	ShaderStage shaders[2] = {
		{ GL_VERTEX_SHADER, "shader.vs" },
		{ GL_FRAGMENT_SHADER, "shader.fs" },
	};
	program = new_program(shaders, 2);

	ShaderStage fullscreen_shaders[2] = {
		{ GL_VERTEX_SHADER, "fullscreen.vs" },
		{ GL_FRAGMENT_SHADER, "fullscreen.fs" },
	};
	fullscreen_program = new_program(fullscreen_shaders, 2);
	glGetUniformLocation(fullscreen_program, "time");

	uint32_t vbo[2], ebo;
	float vertices[] = {
	 0.5f, -0.5f, 0.0f,  // bottom right
	 0.0f,  0.5f, 0.0f,  // top 
	-0.5f, -0.5f, 0.0f,  // bottom left
	};

	float colors[] = {
	 1.0f, 0.5f, 0.0f,  // bottom right
	 0.0f,  1.0f, 0.0f,  // top 
	0.5f, 0.5f, 1.0f,  // bottom left
	};

	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 2,   // first triangle
	};

	glGenBuffers(2, &vbo[0]);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glGenVertexArrays(1, &fullscreen_vao);
}

DWORD WINAPI RenderThread(void* params) {
	// Do stuff.  This will be the first function called on the new thread.
	// When this function returns, the thread goes away.  See MSDN for more details.
	RenderingThreadData * data = params;
	if (!wglMakeCurrent(data->window->dc, data->window->rc))
	{
		show_message("wglMakeCurrent() failed");
		return 1;
	}

	renderer_init();

	BOOL active = TRUE;
	while (active)
	{
		RECT rect;
		if (GetWindowRect(data->window->h_wnd, &rect))
			glClearColor(rect.top / 3000.0, rect.left / 3000.0, 0, 1.0f);
		else
			glClearColor(0, 0, 0, 1);

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(fullscreen_program);
		glUniform1f(time_uniform, time);
		glBindVertexArray(fullscreen_vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

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