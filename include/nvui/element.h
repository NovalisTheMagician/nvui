#pragma once

#include "nvui.h"
#include "util.h"

#include <stdint.h>

typedef struct Window Window;

typedef struct Element Element;

#define E_OF(e) (Element*)(e)

#define UPDATE_HOVER_ENTER (1)
#define UPDATE_HOVER_LEAVE (2)
#define UPDATE_CLICK_PRESS (3)
#define UPDATE_CLICK_RELEASE (4)
#define UPDATE_FOCUS_GAIN (5)
#define UPDATE_FOCUS_LOSE (6)

#define ELEMENT_DESTROY (1 << 30)
#define ELEMENT_DESTROY_DESCENDENT (1 << 31)

typedef enum Message
{
    MSG_LAYOUT,
    MSG_PAINT,
    MSG_UPDATE,
    MSG_DESTROY,

    MSG_LEFT_DOWN,
    MSG_LEFT_UP,
    MSG_MIDDLE_DOWN,
    MSG_MIDDLE_UP,
    MSG_RIGHT_DOWN,
    MSG_RIGHT_UP,
    MSG_MOUSE_MOVE,
    MSG_MOUSE_DRAG,
    MSG_CLICKED,

    MSG_KEY_DOWN,
    MSG_KEY_UP,

    MSG_CHAR,

    MSG_GET_WIDTH,
    MSG_GET_HEIGHT,

    MSG_BUTTON_GET_COLOR,
    MSG_BUTTON_GET_TEXT_COLOR,

    MSG_LABEL_GET_COLOR,

    MSG_PANEL_GET_COLOR,
    MSG_PANEL_GET_BORDER_COLOR,

    MSG_CHECKBOX_GET_COLOR,
    MSG_CHECKBOX_GET_TEXT_COLOR,
    MSG_CHECKBOX_GET_CHECK_COLOR,
    MSG_CHECKBOX_STATE_CHANGE,

    MSG_TEXTFIELD_GET_COLOR,
    MSG_TEXTFIELD_GET_TEXT_COLOR,
    MSG_TEXTFIELD_GET_HIGHLIGHT_COLOR,
    MSG_TEXTFIELD_TEXT_CHANGE,

    MSG_USER
} Message;

NVAPI extern const size_t ElementSize;

typedef int (*MessageHandler)(Element *element, Message message, int di, void *dp);

NVAPI Element* ElementCreate(size_t bytes, Element *parent, uint32_t flags, MessageHandler messageClass);
NVAPI void ElementDestroy(Element *element);

NVAPI int ElementMessage(Element *element, Message message, int di, void *dp);
NVAPI void ElementMove(Element *element, Rectangle bounds, bool alwaysLayout);
NVAPI void ElementRepaint(Element *element, Rectangle *region);
NVAPI Element* ElementFindByPoint(Element *element, int x, int y);

NVAPI void ElementGetCursorPos(Element *element, int *x, int *y);

NVAPI void ElementSetUserHandler(Element *element, MessageHandler userClass);
NVAPI void ElementSetContext(Element *element, void *context);

NVAPI Rectangle ElementGetBounds(Element *element);
NVAPI Window* ElementGetWindow(Element *element);
NVAPI void* ElementGetContext(Element *element);
