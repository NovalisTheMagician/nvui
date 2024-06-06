#pragma once

#include "nvui.h"
#include "element.h"

typedef struct Button
{
    Element e;
    char *text;
    size_t textBytes;
} Button;

NVAPI Button* ButtonCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);

#define LABEL_CENTER (1 << 0)

typedef struct Label
{
    Element e;
    char *text;
    size_t textBytes;
} Label;

NVAPI Label* LabelCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI void LabelSetContent(Label *label, const char *text, ssize_t textBytes);
