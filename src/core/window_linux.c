#include "nvui/window.h"

#include <uchar.h>
#include <string.h>

#include "nvui/private/window.h"
#include "nvui/private/painter.h"
#include "nvui/keys.h"

#if defined __linux__

static bool glFuncsLoaded = false;

static Window* FindWindow(X11Window window)
{
    for(size_t i = 0; i < global.windowCount; ++i)
    {
        if(global.windows[i]->window == window)
            return global.windows[i];
    }
    return NULL;
}

void WindowEndPaint(Window *window, Painter *painter)
{
}

void InvalidateWindow(Window *window)
{
    XClearArea(global.display, window->window, 0, 0, 1, 1, true);
    XFlush(global.display);
}

static bool LoadGLFunctions(void)
{
    if(!glFuncsLoaded)
    {
        int ret = gladLoadGL((GLADloadfunc)glXGetProcAddress);
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

NVAPI void Initialize(void)
{
    global.display = XOpenDisplay(NULL);
    global.windowClosedID = XInternAtom(global.display, "WM_DELETE_WINDOW", 0);

    global.inputMethod = XOpenIM(global.display, NULL, NULL, NULL);
    
    XIMStyles *styles = NULL;
    XGetIMValues(global.inputMethod, XNQueryInputStyle, &styles, NULL);

    for(int i = 0; i < styles->count_styles; ++i)
    {
        XIMStyle thisStyle = styles->supported_styles[i];
        if(thisStyle == (XIMPreeditNothing | XIMStatusNothing))
        {
            global.bestStyle = thisStyle;
            break;
        }
    }
    XFree(styles);

    char *cursorNames[] = { "left_ptr", "xterm", "sb_h_double_arrow", "sb_v_double_arrow", "left_ptr_watch", "wait", "crosshair" };
    for(size_t i = 0; i < NumCursors; ++i)
    {
        global.cursors[i] = XcursorLibraryLoadCursor(global.display, cursorNames[i]);
    }
}

NVAPI void WindowSetCursor(Window *window, CursorShape cursor)
{
    XDefineCursor(global.display, window->window, global.cursors[cursor]);
}

NVAPI Window* WindowCreate(const char *title, int width, int height)
{
    Window *window = (Window*)ElementCreate(sizeof *window, NULL, 0, WindowMessage);
    if(!window) return NULL;

    window->e.window = window;
    window->hovered = &window->e;

    window->width = width;
    window->height = height;

    global.windowCount++;
    global.windows = realloc(global.windows, sizeof(Window*) * global.windowCount);
    global.windows[global.windowCount - 1] = window;
    if(!global.mainWindow)
        global.mainWindow = window;

    int screenId = DefaultScreen(global.display);

    // I assume that every computer in todays age should support the required glx extension to create a modern opengl context
    gladLoaderLoadGLX(global.display, screenId);

    GLint glxAttribs[] =
    {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        
        //GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB, True,

        //GLX_SAMPLE_BUFFERS, True,
        //GLX_SAMPLES, 8,
        None
    };
    int fbcount;
    GLXFBConfig *fbc = glXChooseFBConfig(global.display, screenId, glxAttribs, &fbcount);
    if(!fbc) return NULL;

    window->visual = glXGetVisualFromFBConfig(global.display, fbc[0]);
    if(!window->visual) return NULL;

    XSetWindowAttributes windowAttrib = {};
    windowAttrib.bit_gravity = StaticGravity;
    windowAttrib.border_pixel = BlackPixel(global.display, screenId);
    windowAttrib.background_pixel = WhitePixel(global.display, screenId);
    windowAttrib.override_redirect = True;
    windowAttrib.colormap = XCreateColormap(global.display, DefaultRootWindow(global.display), window->visual->visual, AllocNone);
    window->colormap = windowAttrib.colormap;

    window->window = XCreateWindow(global.display, DefaultRootWindow(global.display), 0, 0, width, height, 0, window->visual->depth, InputOutput, window->visual->visual, CWBackingPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttrib);
    XStoreName(global.display, window->window, title);

    window->inputContext = XCreateIC(global.inputMethod, XNInputStyle, global.bestStyle, XNClientWindow, window->window, XNFocusWindow, window->window, NULL);

    XSelectInput(global.display, window->window, SubstructureNotifyMask | ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask | KeymapStateMask | FocusChangeMask | PropertyChangeMask);

    XSetWMProtocols(global.display, window->window, &global.windowClosedID, 1);
    XMapRaised(global.display, window->window);

    XSizeHints* sizeHints = XAllocSizeHints();
    sizeHints->flags = PMinSize;
    sizeHints->min_width = WINDOW_MIN_WIDTH;
    sizeHints->min_height = WINDOW_MIN_HEIGHT;
    XSetWMNormalHints(global.display, window->window, sizeHints);
    XFree(sizeHints);

    int contextAttribs[] = 
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 6,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifdef _DEBUG
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif

        None
    };

    window->context = glXCreateContextAttribsARB(global.display, fbc[0], NULL, true, contextAttribs);
    if(!window->context) return NULL;
    if(!glXIsDirect(global.display, window->context)) return NULL;

    glXMakeCurrent(global.display, window->window, window->context);

    glXSwapIntervalEXT(global.display, window->window, 1);

    if(!LoadGLFunctions())
        return NULL;

    if(!InitGLData(window))
        return NULL;

    window->firstTimeLayout = true;

    return window;
}

static inline Keycode TranslateKey(unsigned int xkey)
{
    switch(xkey)
    {
    case XK_Left: return KEY_LEFT;
    case XK_Right: return KEY_RIGHT;
    case XK_Control_L: return KEY_LCTRL;
    case XK_Control_R: return KEY_RCTRL;
    case XK_Shift_L: return KEY_LSHIFT;
    case XK_Shift_R: return KEY_RSHIFT;
    }
    return KEY_NONE;
}

static void CloseWindow(Window *window)
{
    ElementDestroy(&window->e);
                    
    glXMakeCurrent(global.display, window->window, window->context);
    Update(window);

    DestroyGLData(window);
    glXDestroyContext(global.display, window->context);
    
    XFree(window->visual);
    XFreeColormap(global.display, window->colormap);
    XDestroyIC(window->inputContext);
    XDestroyWindow(global.display, window->window);
}

NVAPI int MessageLoop(void)
{
    bool isRunning = true;
    while(isRunning)
    {
        XEvent event;
        XNextEvent(global.display, &event);

        switch(event.type)
        {
        case Expose:
            {
                Window *window = FindWindow(event.xexpose.window);
                if(!window) continue;

                glXMakeCurrent(global.display, window->window, window->context);
                if(window->firstTimeLayout)
                {
                    window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
                    window->e.clip = (Rectangle){ .r = window->width, .b = window->height };

                    ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);

                    window->firstTimeLayout = false;
                }

                if(window->buffersNeedResize)
                {
                    ResizeTextures(window);
                    window->buffersNeedResize = false;
                }

                Update(window);
                glDisable(GL_SCISSOR_TEST);
                glClearNamedFramebufferfv(0, GL_COLOR, 0, (float[]){ 0, 1, 0, 1 });
                glBlitNamedFramebuffer(window->glData.framebuffer, 0, 0, 0, window->width, window->height, 0, window->height, window->width, 0, GL_COLOR_BUFFER_BIT, GL_LINEAR);
                glXSwapBuffers(global.display, window->window);
                glEnable(GL_SCISSOR_TEST);
            }
            break;
        case ConfigureNotify:
            {
                Window *window = FindWindow(event.xconfigure.window);
                if(!window) continue;

                if(window->width != event.xconfigure.width || window->height != event.xconfigure.height)
                {
                    window->width = event.xconfigure.width;
                    window->height = event.xconfigure.height;

                    window->e.bounds = (Rectangle){ .r = window->width, .b = window->height };
                    window->e.clip = (Rectangle){ .r = window->width, .b = window->height };

                    ElementMessage(&window->e, MSG_LAYOUT, 0, NULL);
                }
            }
            break;
        case MotionNotify:
            {
                Window *window = FindWindow(event.xmotion.window);
                if(!window) continue;
                window->cursorX = event.xmotion.x;
                window->cursorY = event.xmotion.y;
                WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
            }
            break;
        case LeaveNotify:
            {
                Window *window = FindWindow(event.xcrossing.window);
                if(!window) continue;

                if(!window->pressed)
                {
                    window->cursorX = -1;
                    window->cursorY = -1;
                }
                
                WindowInputEvent(window, MSG_MOUSE_MOVE, 0, NULL);
            }
            break;
        case ButtonPress:
        case ButtonRelease:
            {
                Window *window = FindWindow(event.xbutton.window);
                if(!window) continue;
                window->cursorX = event.xbutton.x;
                window->cursorY = event.xbutton.y;

                if(event.xbutton.button >= 1 && event.xbutton.button <= 3)
                {
                    WindowInputEvent(window, (Message)((event.type == ButtonPress ? MSG_LEFT_DOWN : MSG_LEFT_UP) + event.xbutton.button * 2 - 2), 0, NULL);
                }
            }
            break;
        case KeyPress:
        case KeyRelease:
            {
                Window *window = FindWindow(event.xkey.window);
                if(!window) continue;

                char32_t symbol = 0;
                Status status = 0;
                KeySym keySym = NoSymbol;
                Xutf8LookupString(window->inputContext, (XKeyEvent*)&event, (char*)&symbol, sizeof symbol, &keySym, &status);

                if(status == XLookupChars && event.type == KeyPress)
                {
                    WindowCharEvent(window, MSG_CHAR, symbol, NULL);
                }
                else if(status == XLookupKeySym)
                {
                    Keycode key = TranslateKey(keySym);
                    WindowKeyEvent(window, event.type == KeyPress ? MSG_KEY_DOWN : MSG_KEY_UP, key, NULL);
                }
                else if(status == XLookupBoth)
                {
                    Keycode key = TranslateKey(keySym);
                    WindowKeyEvent(window, event.type == KeyPress ? MSG_KEY_DOWN : MSG_KEY_UP, key, NULL);
                    if(event.type == KeyPress)
                    {
                        WindowCharEvent(window, MSG_CHAR, symbol, NULL);
                    }
                }
                else if(status == XBufferOverflow)
                {
                    // eh?
                }
            }
            break;
        case ClientMessage:
            {
                Window *window = FindWindow(event.xclient.window);
                if(!window) continue;
                if(event.xclient.data.l[0] == global.windowClosedID)
                {
                    bool exit = global.windowCount == 1;
                    if(global.mainWindow == window)
                    {
                        exit = true;
                        for(size_t i = 0; i < global.windowCount; ++i)
                        {
                            if(global.windows[i] == window) continue;
                            CloseWindow(global.windows[i]);
                            i--;
                        }
                    }

                    CloseWindow(window);
                    
                    if(exit)
                        isRunning = false;
                }
            }
            break;
#if 0
        case DestroyNotify:
            {
                isRunning = false;
            }
            break;
#endif
        }
    }

    free(global.windows);

    XCloseIM(global.inputMethod);
    XCloseDisplay(global.display);
    return 0;
}

#endif
