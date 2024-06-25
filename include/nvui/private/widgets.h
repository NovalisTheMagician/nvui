#pragma once

#include <stddef.h>

#include "../widgets.h"
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

typedef struct FlowPanel
{
    Element e;
    Rectangle border;
    int gap;
} FlowPanel;

typedef struct Checkbox
{
    Element e;
    char *text;
    size_t textBytes;
    CheckboxState state;
} Checkbox;

typedef struct Textfield
{
    Element e;
    char *text;
    size_t maxTextBytes, textBytes;
    size_t cursorPos;
    size_t selStart, selEnd;
} Textfield;
