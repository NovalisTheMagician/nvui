#pragma once

#include "nvui.h"
#include "util.h"

#include <stdint.h>

struct Window;
struct Element;

typedef enum Message
{
    MSG_LAYOUT,
    MSG_PAINT,
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
