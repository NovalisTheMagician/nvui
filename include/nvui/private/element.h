#pragma once

#include <stdint.h>

#include "../util.h"
#include "../element.h"

struct Window;

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
