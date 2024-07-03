#pragma once

#include "font.h"
#include "element.h"
#include "../gltypes.h"
#include "../window.h"

#include <cglm/struct.h>

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
#define CursorShape X11CursorShape
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xcursor/Xcursor.h>
#include "glad/glx.h"
#undef Font
#undef Window
#undef CursorShape
#endif

typedef struct GLData
{
    GLuint colorRb, depthRb, framebuffer;
    GLuint shaderProgram, fontProgram, circleProgram, textureLoc;
    GLuint vertexFormat, vertexBuffer;
    Vertex *mappedVertexBuffer;
    GLuint whiteTexture, fontTexture;
    GLuint matrixBuffer, paintPropBuffer, circleBuffer;
} GLData;

typedef struct MatrixData
{
    mat4s projection;
} MatrixData;

typedef struct PaintPropsData
{
    vec4s tint;
} PaintPropsData;

typedef struct CirclePropsData
{
    vec2s center, radius;
} CirclePropsData;

typedef struct Window
{
    Element e;
    int width, height;
    Rectangle updateRegion;
    int cursorX, cursorY;
    Element *hovered, *pressed, *focused;
    int pressedButton;

    GLData glData;
    mat4s projection;
    bool buffersNeedResize;
    Font fonts[NumVariants];

#ifdef _WIN32
    HCURSOR currentCursor;

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
    XIC inputContext;
#endif
} Window;
