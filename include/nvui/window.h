#pragma once

#include <stdlib.h>
#include "nvui.h"
#include "element.h"
#include "font.h"

typedef enum FontVariant
{
    Sans,
    Serif,
    Mono
} FontVariant;

struct Window;
typedef struct Window Window;

NVAPI void Initialize(void);
NVAPI Window* WindowCreate(const char *title, int width, int height);
NVAPI int MessageLoop(void);
NVAPI Element* WindowGetRootElement(Window *window);
NVAPI Font* WindowGetFontVariant(Window *window, FontVariant variant);
