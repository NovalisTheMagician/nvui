#pragma once

#include "nvui.h"
#include "element.h"

#define ELEMENT_V_FILL (1 << 16)
#define ELEMENT_H_FILL (1 << 17)

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

#define PANEL_GRAY (1 << 1)
#define PANEL_WHITE (1 << 2)
#define PANEL_HORIZONTAL (1 << 0)
struct FlowPanel;
typedef struct FlowPanel FlowPanel;

NVAPI FlowPanel* FlowPanelCreate(Element *parent, uint32_t flags);
NVAPI void FlowPanelSetGap(FlowPanel *panel, int gap);
NVAPI void FlowPanelSetBorder(FlowPanel *panel, Rectangle border);
