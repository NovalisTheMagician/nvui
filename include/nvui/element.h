#pragma once

#include "nvui.h"
#include "util.h"

#include <stdint.h>

struct Window;
struct Element;

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
    MSG_USER
} Message;

typedef int (*MessageHandler)(struct Element *element, Message message, int di, void *dp);

typedef struct Element
{
    uint32_t flags;
    uint32_t childCount;
    Rectangle bounds, clip;
    struct Element *parent;
    struct Element **children;
    struct Window *window;
    void *user;
    MessageHandler messageClass, messageUser;
} Element;

NVAPI Element* ElementCreate(size_t bytes, Element *parent, uint32_t flags, MessageHandler messageClass);
NVAPI int ElementMessage(Element *element, Message message, int di, void *dp);
NVAPI void ElementMove(Element *element, Rectangle bounds, bool alwaysLayout);
NVAPI void ElementRepaint(Element *element, Rectangle *region);
NVAPI Element* ElementFindByPoint(Element *element, int x, int y);
