#pragma once

#include "nvui.h"
#include "element.h"
#include "color.h"

#include <stdlib.h>

#ifdef _WIN32
#define Rectangle W32Rectangle
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef Rectangle
#elif defined(__linux__)
#define Window X11Window
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#undef Window
#endif

typedef struct Window
{
    Element e;
    char *title;
    int width, height;
    Rectangle updateRegion;
    Color windowColor;

#ifdef _WIN32
    HWND hwnd;
    HGLRC hglrc;
    HDC hdc;
    bool trackingLeave;
#elif __linux__
    X11Window window;
#endif
} Window;

typedef struct GlobalState
{
    Window **windows;
    size_t windowCount;

#ifdef __linux__
    Display *display;
    Visual *visual;
    Atom windowClosedID;
#endif
} GlobalState;

NVAPI void Initialize(void);
NVAPI Window* WindowCreate(const char *title, int width, int height);
NVAPI int MessageLoop(void);
