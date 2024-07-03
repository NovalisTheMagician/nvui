#pragma once

#include "nvui.h"
#include "element.h"
#include "font.h"

#define WINDOW_NO_RESIZE (1 << 0)

typedef enum FontVariant
{
    Sans,
    Serif,
    Mono,
    NumVariants,
    DefaultVariant = Sans
} FontVariant;

typedef enum CursorShape
{
    Arrow,
    Bar,
    Horizontal,
    Vertical,
    ArrowLoading,
    Loading,
    Crosshair,
    NumCursors
} CursorShape;

typedef struct Window Window;

NVAPI void Initialize(void);
NVAPI int MessageLoop(void);
NVAPI void SetMainWindow(Window *window);

NVAPI Window* WindowCreate(const char *title, int width, int height);
NVAPI Element* WindowGetRootElement(Window *window);
NVAPI Font* WindowGetFontVariant(Window *window, FontVariant variant);

NVAPI Element* WindowGetPressed(Window *window);
NVAPI Element* WindowGetHovered(Window *window);
NVAPI Element* WindowGetFocused(Window *window);

NVAPI void WindowSetFocused(Window *window, Element *element);

NVAPI void WindowGetCursorPos(Window *window, int *x, int *y);
NVAPI void WindowSetCursor(Window *window, CursorShape cursor);
