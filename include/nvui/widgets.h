#pragma once

#include "nvui.h"
#include "element.h"

struct Button;
typedef struct Button Button;

NVAPI Button* ButtonCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI Element* ButtonGetElement(Button *button);

#define LABEL_CENTER (1 << 0)
struct Label;
typedef struct Label Label;

NVAPI Label* LabelCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI void LabelSetContent(Label *label, const char *text, ssize_t textBytes);
NVAPI Element* LabelGetElement(Label *label);
