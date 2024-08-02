#pragma once

#include "font.h"
#include "element.h"
#include "../gltypes.h"
#include "../window.h"
#include "menu.h"

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
#include "../../glad/glx.h"
#undef Font
#undef Window
#undef CursorShape
#endif

typedef struct GlobalState
{
    Window **windows;
    size_t windowCount;
    Window *mainWindow;
#ifdef WIN32
    HCURSOR cursors[NumCursors];
#elif defined(__linux__)
    Cursor cursors[NumCursors];
    Display *display;
    Atom windowClosedID;
    XIM inputMethod;
    XIMStyle bestStyle;
#endif
} GlobalState;
extern GlobalState global;

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
    bool shiftDown, ctrlDown, altDown;

    GLData glData;
    mat4s projection;
    bool buffersNeedResize;
    Font fonts[NumVariants];

    Menu *menu;
    MenuItem *itemHovered;

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

#define WINDOW_MIN_WIDTH 200
#define WINDOW_MIN_HEIGHT 200
#define BUFFER_SIZE 1024 * 64 //64k vertices as a draw buffer. that should support complex drawing operations
#define MENUBAR_HEIGHT 24
#define MENUITEM_MARGIN 6

bool InitGLData(Window *window);
void Update(Window *window);
void RemoveWindow(Window *window);
void DestroyGLData(Window *window);
void ResizeTextures(Window *window);

int WindowMessage(Element *element, Message message, int di, void *dp);
void WindowInputEvent(Window *window, Message message, int di, void *dp);
void WindowKeyEvent(Window *window, Message message, int di, void *dp);
void WindowCharEvent(Window *window, Message message, int di, void *dp);
void GLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user);
