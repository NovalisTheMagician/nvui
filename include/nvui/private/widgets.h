#pragma once

#include <stddef.h>

#include "element.h"

typedef struct Button
{
    Element e;
    char *text;
    size_t textBytes;
} Button;

typedef struct Label
{
    Element e;
    char *text;
    size_t textBytes;
} Label;
