#pragma once

#include "nvui.h"

typedef struct Window
{
    char *title;
} Window;

Window* NVAPI CreateWindow(const char *title);
