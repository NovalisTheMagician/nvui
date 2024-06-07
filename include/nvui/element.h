#pragma once

#include "nvui.h"
#include "util.h"

#include <stdint.h>

struct Window;
typedef struct Window Window;

struct Element;
typedef struct Element Element;

#define UPDATE_HOVERED (1)
#define UPDATE_PRESSED (2)

typedef enum Message
{
    MSG_LAYOUT,
    MSG_PAINT,
    MSG_UPDATE,

    MSG_LEFT_DOWN,
    MSG_LEFT_UP,
    MSG_MIDDLE_DOWN,
    MSG_MIDDLE_UP,
    MSG_RIGHT_DOWN,
    MSG_RIGHT_UP,
    MSG_MOUSE_MOVE,
    MSG_MOUSE_DRAG,
    MSG_CLICKED,

    MSG_BUTTON_GET_COLOR,

    MSG_USER
} Message;

NVAPI extern const size_t ElementSize;

typedef int (*MessageHandler)(Element *element, Message message, int di, void *dp);

NVAPI Element* ElementCreate(size_t bytes, Element *parent, uint32_t flags, MessageHandler messageClass);
NVAPI int ElementMessage(Element *element, Message message, int di, void *dp);
NVAPI void ElementMove(Element *element, Rectangle bounds, bool alwaysLayout);
NVAPI void ElementRepaint(Element *element, Rectangle *region);
NVAPI Element* ElementFindByPoint(Element *element, int x, int y);
NVAPI void ElementSetUserHandler(Element *element, MessageHandler userClass);
NVAPI Rectangle ElementGetBounds(Element *element);
NVAPI Window* ElementGetWindow(Element *element);
