#include "nvui/window.h"

#include <uchar.h>

#include "nvui/private/window.h"
#include "nvui/private/painter.h"
#include "nvui/keys.h"

#ifdef _WIN32
#define WINDOW_CLASS L"NVWINDOW"

static bool glFuncsLoaded = false;

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

    // I assume that every computer in todays age should support the required wgl extension to create a modern opengl context
    gladLoaderLoadWGL(dc);

    if(!GLAD_WGL_ARB_create_context || !GLAD_WGL_ARB_create_context_profile || !GLAD_WGL_ARB_pixel_format || !GLAD_WGL_EXT_swap_control)
    {
        fprintf(stderr, "This system does not support the required wgl extensions\n");
        return false;
    }

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

#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLDebugCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif

    return true;
}

void WindowEndPaint(Window *window, Painter *painter)
{
}

static Keycode TranslateKey(WPARAM wParam)
{
    switch(wParam)
    {
    case VK_LEFT: return KEY_LEFT;
    case VK_RIGHT: return KEY_RIGHT;
    case VK_UP: return KEY_UP;
    case VK_DOWN: return KEY_DOWN;
    case VK_RETURN: return KEY_ENTER;
    case VK_BACK: return KEY_BACKSPACE;
    case VK_LCONTROL: return KEY_LCTRL;
    case VK_RCONTROL: return KEY_RCTRL;
    //case VK_LSHIFT: return KEY_LSHIFT;
    //case VK_RSHIFT: return KEY_RSHIFT;
    //case VK_LMENU: return KEY_LALT;
    //case VK_RMENU: return KEY_RALT;
    //case VK_LWIN: return KEY_LMETA;
    //case VK_RWIN: return KEY_RMETA;

    case VK_SHIFT: return KEY_LSHIFT;
    case VK_CONTROL: return KEY_LCTRL;
    case VK_MENU: return KEY_LALT;

    case VK_APPS: return KEY_MENU;
    case VK_CAPITAL: return KEY_CAPSLOCK;
    case VK_ESCAPE: return KEY_ESCAPE;
    case VK_TAB: return KEY_TAB;
    case VK_DELETE: return KEY_DELETE;
    case VK_INSERT: return KEY_INSERT;
    case VK_HOME: return KEY_HOME;
    case VK_END: return KEY_END;
    case VK_PRIOR: return KEY_PAGEUP;
    case VK_NEXT: return KEY_PAGEDOWN;
    // finish the rest of the mapping
    }
    return KEY_NONE;
}

void InvalidateWindow(Window *window)
{
    InvalidateRect(window->hwnd, NULL, false);
}

NVAPI void WindowSetCursor(Window *window, CursorShape cursor)
{
    window->currentCursor = global.cursors[cursor];
    PostMessageW(window->hwnd, WM_SETCURSOR, 0, 0);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window *window = (Window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    if(!window) return DefWindowProcW(hwnd, message, wParam, lParam);

    switch(message)
    {
    case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        }
        break;
    case WM_DESTROY:
        {
            bool exit = global.windowCount == 1;
            if(global.mainWindow == window)
            {
                exit = true;
                for(size_t i = 0; i < global.windowCount; ++i)
                {
                    ShowWindow(global.windows[i]->hwnd, SW_HIDE);
                }

                for(size_t i = 0; i < global.windowCount; ++i)
                {
                    if(global.windows[i] == window) continue;
                    DestroyWindow(global.windows[i]->hwnd);
                    i--;
                }
            }

            ElementDestroy(&window->e);
            
            wglMakeCurrent(window->hdc, window->hglrc);
            Update(window);

            DestroyGLData(window);
            wglDeleteContext(window->hglrc);

            if(exit)
                PostQuitMessage(0);
        }
        break;
    case WM_SIZE:
        {
            RECT client;
            GetClientRect(hwnd, &client);
            if(client.right > 0 && client.bottom > 0)
            {
                window->width = client.right;
                window->height = client.bottom;
                window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
                window->e.clip = (Rectangle){ .r = window->width, .b = window->height };
                ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);
                //InvalidateRect(element->window->hwnd, NULL, false);
            }
        }
        break;
    case WM_MOUSEMOVE:
        {
            if(!window->trackingLeave)
            {
                window->trackingLeave = true;
                TRACKMOUSEEVENT leave = 
                {
                    .cbSize = sizeof leave,
                    .dwFlags = TME_LEAVE,
                    .hwndTrack = window->hwnd
                };
                TrackMouseEvent(&leave);
            }

            POINT cursor = {};
            GetCursorPos(&cursor);
            ScreenToClient(window->hwnd, &cursor);
            if(!(window->prevCursorX == cursor.x && window->prevCursorY == cursor.y))
            {
                window->prevCursorX = cursor.x;
                window->prevCursorY = cursor.y;
                window->cursorX = cursor.x;
                window->cursorY = cursor.y;
                WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
            }
        }
        break;
    case WM_MOUSELEAVE:
        {
            window->trackingLeave = false;

            if(!window->pressed)
            {
                window->cursorX = -1;
                window->cursorY = -1;
            }

            WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
        }
        break;
    case WM_LBUTTONDOWN:
        {
            SetCapture(window->hwnd);
            WindowInputEvent(window, MSG_LEFT_DOWN, 0, NULL);
        }
        break;
    case WM_LBUTTONUP:
        {
            if(window->pressedButton == 1) ReleaseCapture();
            WindowInputEvent(window, MSG_LEFT_UP, 0, NULL);
        }
        break;
    case WM_MBUTTONDOWN:
        {
            SetCapture(window->hwnd);
            WindowInputEvent(window, MSG_MIDDLE_DOWN, 0, NULL);
        }
        break;
    case WM_MBUTTONUP:
        {
            if(window->pressedButton == 2) ReleaseCapture();
            WindowInputEvent(window, MSG_MIDDLE_UP, 0, NULL);
        }
        break;
    case WM_RBUTTONDOWN:
        {
            SetCapture(window->hwnd);
            WindowInputEvent(window, MSG_RIGHT_DOWN, 0, NULL);
        }
        break;
    case WM_RBUTTONUP:
        {
            if(window->pressedButton == 3) ReleaseCapture();
            WindowInputEvent(window, MSG_RIGHT_UP, 0, NULL);
        }
        break;
    case WM_KEYDOWN:
    case WM_KEYUP:
        {
            int key = TranslateKey(wParam);
            WindowKeyEvent(window, message == WM_KEYDOWN ? MSG_KEY_DOWN : MSG_KEY_UP, key, NULL);
        }
        break;
    case WM_CHAR:
        {
            char32_t ch = 0;
            WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)&wParam, 1, (char*)&ch, sizeof ch, NULL, NULL);
            WindowCharEvent(window, MSG_CHAR, ch, NULL);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(hwnd, &paint);
            wglMakeCurrent(window->hdc, window->hglrc);
            if(window->buffersNeedResize)
            {
                ResizeTextures(window);
                window->buffersNeedResize = false;
            }
            Update(window);
            
            glDisable(GL_SCISSOR_TEST);
            glClearNamedFramebufferfv(0, GL_COLOR, 0, (float[]){ 0, 1, 1, 1 });
            glBlitNamedFramebuffer(window->glData.framebuffer, 0, 0, 0, window->width, window->height, 0, window->height, window->width, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            SwapBuffers(window->hdc);
            glEnable(GL_SCISSOR_TEST);
            EndPaint(hwnd, &paint);
        }
        break;
    case WM_SETCURSOR:
        {
            if(window->currentCursor != global.cursors[Arrow])
            {
                SetCursor(window->currentCursor);
                return 1;
            }
            else
                return DefWindowProcW(hwnd, message, wParam, lParam);
        }
        break;
    case WM_GETMINMAXINFO:
        {
            RECT windowRect = { .right = WINDOW_MIN_WIDTH, .bottom = WINDOW_MIN_HEIGHT };
            AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
            MINMAXINFO *mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = windowRect.right - windowRect.left;
            mmi->ptMinTrackSize.y = windowRect.bottom - windowRect.top;
        }
        break;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    return 0;
}

NVAPI void Initialize(void)
{
    LoadPreGLFunctions();

    WNDCLASSEXW windowClass = 
    {
        .cbSize = sizeof windowClass,
        .lpfnWndProc = WndProc,
        .hCursor = LoadCursorW(NULL, IDC_ARROW),
        .hIcon = LoadIconW(NULL, IDI_APPLICATION),
        .hInstance = GetModuleHandleW(NULL),
        .style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW,
        .lpszClassName = WINDOW_CLASS
    };
    RegisterClassExW(&windowClass);

    LPWSTR cursorNames[] = { IDC_ARROW, IDC_IBEAM, IDC_SIZEWE, IDC_SIZENS, IDC_APPSTARTING, IDC_WAIT, IDC_CROSS };
    for(size_t i = 0; i < NumCursors; ++i)
    {
        global.cursors[i] = LoadCursorW(NULL, cursorNames[i]);
    }
}

NVAPI Window* WindowCreate(const char *title, int width, int height)
{
    Window *window = (Window*)ElementCreate(sizeof *window, NULL, 0, WindowMessage);
    if(!window) return NULL;

    window->currentCursor = global.cursors[Arrow];
    window->e.window = window;
    window->hovered = &window->e;

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof(Window*) * global.windowCount);
    global.windows[global.windowCount - 1] = window;
    if(!global.mainWindow)
        global.mainWindow = window;

    RECT windowRect = { .right = width, .bottom = height };
    AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

    size_t wideTitleSize = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    wchar_t *wideTitle = calloc(wideTitleSize, sizeof *wideTitle);
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wideTitle, wideTitleSize);

    window->hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, WINDOW_CLASS, wideTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, GetModuleHandleW(NULL), NULL);
    free(wideTitle);
    if(!window->hwnd) return NULL;
    SetWindowLongPtrW(window->hwnd, GWLP_USERDATA, (LONG_PTR)window);

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

    if(!LoadGLFunctions())
        return NULL;

    if(!InitGLData(window))
        return NULL;

    ShowWindow(window->hwnd, SW_SHOW);
    PostMessageW(window->hwnd, WM_SIZE, 0, 0);

    return window;
}

NVAPI int MessageLoop(void)
{
    MSG message = {};

    while(GetMessageW(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    free(global.windows);

    return message.wParam;
}

#endif
