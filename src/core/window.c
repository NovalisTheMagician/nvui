#include "nvui/window.h"

#include <stdint.h>
#include <stdlib.h>
#include <glad2/gl.h>

#include "nvui/painter.h"

#include <stdio.h>

GlobalState global = {};
static bool glFuncsLoaded = false;

static int WindowMessage(Element *element, Message message, int di, void *dp)
{
    if(message == MSG_LAYOUT)
    {
        glViewport(0, 0, element->window->width, element->window->height);
        if(element->childCount)
        {
            ElementMove(element->children[0], element->bounds, false);
            ElementRepaint(element, NULL);
        }
    }
    return 0;
}

static void WindowEndPaint(Window *window, Painter *painter);

static void ElementPaint(Element *element, Painter *painter)
{
    Rectangle clip = RectangleIntersection(element->clip, painter->clip);
    if(!RectangleValid(clip)) return;

    painter->clip = clip;
    glScissor(clip.l, clip.t, clip.r - clip.l, clip.b - clip.t);
    ElementMessage(element, MSG_PAINT, 0, painter);

    for(size_t i = 0; i < element->childCount; ++i)
    {
        painter->clip = clip;
        ElementPaint(element->children[i], painter);
    }
}

static void Update(void)
{
    for(size_t i = 0; i < global.windowCount; ++i)
    {
        Window *window = global.windows[i];
        glClearColor(window->windowColor.r, window->windowColor.g, window->windowColor.b, window->windowColor.a);
        glClear(GL_COLOR_BUFFER_BIT);
        if(RectangleValid(window->updateRegion))
        {
            Painter painter;
            painter.width = window->width;
            painter.height = window->height;
            painter.clip = RectangleIntersection((Rectangle){ .r = window->width, .b = window->height }, window->updateRegion);
            ElementPaint(&window->e, &painter);
            WindowEndPaint(window, &painter);
            window->updateRegion = (Rectangle){ 0 };
            //if(glFuncsLoaded)
                //glScissor(0, 0, window->width, window->height);
        }
    }
}

#ifdef _WIN32
#include <nvui/gl/wglext.h>

#define WINDOW_CLASS "NVWINDOW"

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

static bool LoadPreGLFunctions(void)
{
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    if(!dummy) return false;

    HDC dc = GetDC(dummy);
    if(!dc) return false;

    PIXELFORMATDESCRIPTOR pfd = 
    {
        .nSize = sizeof pfd,
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24
    };

    int format = ChoosePixelFormat(dc, &pfd);
    if(!format) return false;

    int ok = DescribePixelFormat(dc, format, sizeof pfd, &pfd);
    if(!ok) return false;

    if(!SetPixelFormat(dc, format, &pfd)) return false;

    HGLRC rc = wglCreateContext(dc);
    if(!rc) return false;

    ok = wglMakeCurrent(dc, rc);
    if(!ok) return false;

    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = (void*)wglGetProcAddress("wglGetExtensionsStringARB");
    if(!wglGetExtensionsStringARB) return false;

    // I assume that every computer in todays age should support the required wgl extension to create a modern opengl context
    wglChoosePixelFormatARB = (void*)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (void*)wglGetProcAddress("wglCreateContextAttribsARB");
    wglSwapIntervalEXT = (void*)wglGetProcAddress("wglSwapIntervalEXT");

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);

    return true;
}

static bool LoadGLFunctions(void)
{
    if(!glFuncsLoaded)
    {
        int ret = gladLoadGL((GLADloadfunc)wglGetProcAddress);
        if(ret) glFuncsLoaded = true;
        else return ret;
    }

    glEnable(GL_SCISSOR_TEST);

    return true;
}

static void WindowEndPaint(Window *window, Painter *painter)
{
    SwapBuffers(window->hdc);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window *window = (Window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    if(!window) return DefWindowProcA(hwnd, message, wParam, lParam);

    switch(message)
    {
    case WM_CLOSE:
        {
            PostQuitMessage(0);
        }
        break;
    case WM_SIZE:
        {
            RECT client;
            GetClientRect(hwnd, &client);
            window->width = client.right;
            window->height = client.bottom;
            window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
            window->e.clip = (Rectangle){ .r = window->width, .b = window->height };
            ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);
            Update();
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
		    BeginPaint(hwnd, &paint);

            //Update();
            SwapBuffers(window->hdc);

            EndPaint(hwnd, &paint);
        }
        break;
    default:
        return DefWindowProcA(hwnd, message, wParam, lParam);
    }

    return 0;
}

NVAPI void Initialize(void)
{
    LoadPreGLFunctions();

    WNDCLASSEXA windowClass = 
    {
        .cbSize = sizeof windowClass,
        .lpfnWndProc = WndProc,
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .hIcon = LoadIconA(NULL, IDI_APPLICATION),
        .lpszClassName = WINDOW_CLASS
    };
    RegisterClassExA(&windowClass);
}

NVAPI Window* WindowCreate(const char *title, int width, int height)
{
    Window *window = (Window*)ElementCreate(sizeof *window, NULL, 0, WindowMessage);
    if(!window) return NULL;

    window->e.window = window;
    window->windowColor = (Color){ .r = 1, .b = 1, .a = 1 };

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof *global.windows * global.windowCount);
    global.windows[global.windowCount - 1] = window;

    window->hwnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW, WINDOW_CLASS, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, NULL, NULL);
    if(!window->hwnd) return NULL;
    SetWindowLongPtrA(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

    window->hdc = GetDC(window->hwnd);
    if(!window->hdc) return NULL;

    int pixelattrib[] = 
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 24,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,

        //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,

        //WGL_SAMPLE_BUFFERS_ARB, 1,
        //WGL_SAMPLES_ARB, 4, // 4x MSAA

        0
    };

    int format;
    UINT formats;
    if(!wglChoosePixelFormatARB(window->hdc, pixelattrib, NULL, 1, &format, &formats)) return NULL;

    PIXELFORMATDESCRIPTOR pfd = { .nSize = sizeof pfd };
    int ok = DescribePixelFormat(window->hdc, format, sizeof pfd, &pfd);
    if(!ok) return NULL;

    if(!SetPixelFormat(window->hdc, format, &pfd)) return NULL;

    int glattrib[] = 
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#if _DEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };

    window->hglrc = wglCreateContextAttribsARB(window->hdc, NULL, glattrib);
    if(!window->hglrc) return NULL;

    ok = wglMakeCurrent(window->hdc, window->hglrc);
    if(!ok) return NULL;

    wglSwapIntervalEXT(1);

    LoadGLFunctions();

    ShowWindow(window->hwnd, SW_SHOW);
    PostMessageA(window->hwnd, WM_SIZE, 0, 0);

    return window;
}

NVAPI int MessageLoop(void)
{
    MSG message = {};

    while(GetMessageA(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return message.wParam;
}

#elif defined __linux__
#include <GL/glx.h>

NVAPI void Initialize(void)
{
    
}

NVAPI Window* WindowCreate(const char *title, int width, int height)
{
    
}

NVAPI int MessageLoop(void)
{
    
}

#endif
