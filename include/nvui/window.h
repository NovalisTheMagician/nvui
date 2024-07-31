#pragma once

#include "nvui.h"
#include "element.h"
#include "font.h"
#include "menu.h"

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

#define META_KEY_STATE(dp) ((uint16_t)(uintptr_t)dp)
#define META_SHIFT_DOWN(dp) ((META_KEY_STATE(dp) >> 1) & 1)
#define META_CTRL_DOWN(dp) ((META_KEY_STATE(dp) >> 2) & 1)
#define META_ALT_DOWN(dp) ((META_KEY_STATE(dp) >> 0) & 1)

typedef struct Window Window;

NVAPI void Initialize(void);
NVAPI int MessageLoop(void);
NVAPI void SetMainWindow(Window *window);

NVAPI void WindowSetMenu(Window *window, Menu *menu);

NVAPI Window* WindowCreate(const char *title, int width, int height);
NVAPI Element* WindowGetRootElement(Window *window);
NVAPI Font* WindowGetFontVariant(Window *window, FontVariant variant);

NVAPI Element* WindowGetPressed(Window *window);
NVAPI Element* WindowGetHovered(Window *window);
NVAPI Element* WindowGetFocused(Window *window);

NVAPI void WindowSetFocused(Window *window, Element *element);

NVAPI void WindowGetCursorPos(Window *window, int *x, int *y);
NVAPI void WindowSetCursor(Window *window, CursorShape cursor);
