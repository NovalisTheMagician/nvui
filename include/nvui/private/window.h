#pragma once

#include "font.h"
#include "element.h"
#include "../gltypes.h"

#ifdef _WIN32
#define Rectangle W32Rectangle
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../../glad/wgl.h"
#undef Rectangle
#elif defined(__linux__)
#define Window X11Window
#define Font X11Font
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include "glad/glx.h"
#undef Font
#undef Window
#endif

typedef struct GLData
{
    GLuint colorRb, depthRb, framebuffer;
    GLuint shaderProgram, fontProgram, projectionLoc, tintLoc, textureLoc;
    GLuint vertexFormat, vertexBuffer;
    Vertex *mappedVertexBuffer;
    GLuint whiteTexture, fontTexture;
} GLData;

typedef struct Window
{
    Element e;
    int width, height;
    Rectangle updateRegion;
    int cursorX, cursorY;
    Element *hovered, *pressed;
    int pressedButton;

    GLData glData;
    mat4s projection;
    bool buffersNeedResize;
    Font defaultFont;

#ifdef _WIN32
    HWND hwnd;
    HGLRC hglrc;
    HDC hdc;
    bool trackingLeave;
    int prevCursorX, prevCursorY;
#elif __linux__
    X11Window window;
    GLXContext context;
    XVisualInfo *visual;
    Colormap colormap;
    bool firstTimeLayout;
#endif
} Window;
