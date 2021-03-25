#include <string>
#include <iostream>
#include <Windows.h>
//#include <gl/GL.h>

#define in _In_
#define inOp _In_opt_

namespace gl
{
	using GLvoid = VOID;
	using GLbool = INT8;
	using GLbyte = INT8;
	using GLubyte = UINT8;
	using GLShort = INT16;
	using GLushort = UINT16;
	using GLint = INT32;
	using GLuint = INT32;
	using GLfixed = INT32;
	using GLint64 = INT64;
	using GLuint64 = UINT64;
	using GLsizei = INT32;
	using GLenum = INT32;
	using GLintptr = GLint *;
	using GLsizeiptr = GLsizei *;
	using GLsync = GLvoid *;
	using GLbitfield = INT32;
	using GLfloat = FLOAT;
	using GLclampf = FLOAT;
	using GLdouble = DOUBLE;
	using GLclampd = DOUBLE;

	enum Constants: GLenum
	{
		COLOR_BUFFER_BIT = 0x00004000,
	};
	
	#define GL_FUNC(name, ret, ...) using name##_t = WINGDIAPI ret (APIENTRY *)(##__VA_ARGS__); name##_t name

	GL_FUNC(Clear, GLvoid, in GLbitfield mask);
	GL_FUNC(ClearColor, GLvoid, in GLclampf red, in GLclampf green, in GLclampf blue, in GLclampf alpha);
	GL_FUNC(Flush, GLvoid, in GLvoid);
	GL_FUNC(Viewport, GLvoid, in GLint x, in GLint y, GLsizei width, GLsizei height);
	GL_FUNC(GetError, GLenum, in GLvoid);
	GL_FUNC(GetString, const GLubyte *, in GLenum);

	#undef GL_FUNC

	#define STR(arg) #arg
	#define GL_FUNC(name) name = (name##_t)GetProcAddress(glLib, STR(gl##name)); if (name == NULL) { MessageBox(NULL, TEXT(STR(Failed to load function #name)), TEXT("CppGL Error"), MB_OK); return FALSE; }
	#define GL_FUNC_ALIAS(name, libName) name = (name##_t)GetProcAddress(glLib, #libName); if (name == NULL) { MessageBox(NULL, TEXT(STR(Failed to load function #name)), TEXT("CppGL Error"), MB_OK); return FALSE; }
	GLbool Init()
	{
		HMODULE glLib = LoadLibrary(TEXT("OpenGL32"));
		if (glLib == NULL)
		{
			MessageBox(NULL, TEXT("Failed to load OpenGL library"), TEXT("CppGL Error"), MB_OK);
			return FALSE;
		}

		GL_FUNC(Clear);
		GL_FUNC(ClearColor);
		GL_FUNC(Flush);
		GL_FUNC(Viewport);
		GL_FUNC(GetError);
		GL_FUNC(GetString);

		return TRUE;
	}
	#undef STR
	#undef GL_FUNC

	HWND MakeWindow(in PCTSTR title, in INT x, in INT y, in INT width, in INT height, in BYTE ty, in DWORD flags, in WNDPROC winProc)
	{
		INT pixelFormat;
		HDC context;
		HWND win;
		WNDCLASS clazz;
		PIXELFORMATDESCRIPTOR pfd = {};
		static HMODULE module;
		static PCTSTR className = TEXT("CppGL");

		if (module == NULL)
		{
			module = GetModuleHandle(NULL);
			clazz.style = CS_OWNDC;
			clazz.lpfnWndProc = winProc;
			clazz.cbClsExtra = 0;
			clazz.cbWndExtra = 0;
			clazz.hInstance = module;
			clazz.hIcon = LoadIcon(NULL, IDI_WINLOGO);
			clazz.hCursor = LoadCursor(NULL, IDC_ARROW);
			clazz.hbrBackground = NULL;
			clazz.lpszMenuName = NULL;
			clazz.lpszClassName = className;

			if (!RegisterClass(&clazz))
			{
				MessageBox(NULL, TEXT("Cannot register window class"), TEXT("Fatal Error!"), MB_OK);
				return NULL;
			}
		}

		win = CreateWindow(className, title, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, x, y, width, height, NULL, NULL, module, NULL);
		if (win == NULL)
		{
			MessageBox(NULL, TEXT("Cannot create window"), TEXT("Fatal Error!"), MB_OK);
			return NULL;
		}

		context = GetDC(win);

		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | flags;
		pfd.iPixelType = ty;
		pfd.cColorBits = 32;

		pixelFormat = ChoosePixelFormat(context, &pfd);
		if (pixelFormat == 0)
		{
			MessageBox(NULL, TEXT("Cannot format the specified"), TEXT("Fatal Error!"), MB_OK);
			return NULL;
		}

		DescribePixelFormat(context, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
		ReleaseDC(win, context);
		return win;
	}
}

LRESULT WINAPI MyWinProc(HWND win, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT ps;

	switch (msg)
	{
	case WM_PAINT:
	{
		//gl::Clear(gl::COLOR_BUFFER_BIT);
		//std::wstring err = std::to_wstring(gl::GetError());
		//err += '\n';
		//OutputDebugString(err.c_str());
		//gl::Flush();
		return 0;
	}
	case WM_SIZE:
		//gl::Viewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(win, WM_PAINT, 0, 0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(win, msg, wParam, lParam);
}

INT APIENTRY WinMain(in HMODULE module, inOp HMODULE prevModule, in LPSTR cmdLine, in BOOL cmdShow)
{
	HWND win = gl::MakeWindow(TEXT("Test"), 0, 0, 256, 256, PFD_TYPE_RGBA, 0, MyWinProc);
	if (win == NULL)
		exit(EXIT_FAILURE);

	HDC context = GetDC(win);
	HGLRC glContext = wglCreateContext(context);
	wglMakeCurrent(context, glContext);

	if (gl::Init() == FALSE)
		exit(EXIT_FAILURE);

	ShowWindow(win, cmdShow);
	//gl::ClearColor(0.25, 0.25, 0.25, 1.0);

	gl::GLenum glErr;
	while (glErr = gl::GetError())
	{
		std::wstring err = std::to_wstring(glErr);
		err += '\n';
		OutputDebugString(err.c_str());
	}

	MSG msg;
	while (GetMessage(&msg, win, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Cleanup, cleanup, everybody everywhere!
	wglMakeCurrent(NULL, NULL);
	ReleaseDC(win, context);
	wglDeleteContext(glContext);
	DestroyWindow(win);
	return msg.wParam;
}