#pragma once

#include "nvui.h"
#include "element.h"

#define ELEMENT_V_FILL (1 << 16)
#define ELEMENT_H_FILL (1 << 17)

#define BUTTON_BORDER (1 << 0)
typedef struct Button Button;

NVAPI Button* ButtonCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI Element* ButtonGetElement(Button *button);

#define LABEL_CENTER (1 << 0)
typedef struct Label Label;

NVAPI Label* LabelCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI void LabelSetContent(Label *label, const char *text, ssize_t textBytes);
NVAPI Element* LabelGetElement(Label *label);

#define FLOWPANEL_HORIZONTAL (1 << 0)
#define PANEL_BORDER (1 << 1)
#define PANEL_BORDER_3D (1 << 2)
typedef struct FlowPanel FlowPanel;

NVAPI FlowPanel* FlowPanelCreate(Element *parent, uint32_t flags);
NVAPI void FlowPanelSetGap(FlowPanel *panel, int gap);
NVAPI void FlowPanelSetBorder(FlowPanel *panel, Rectangle border);

#define CHECKBOX_CHECK_CROSS (1 << 0)
#define CHECKBOX_TRISTATE (1 << 1)
#define CHECKBOX_BORDER (1 << 2)
typedef enum CheckboxState
{
    Unchecked,
    Checked,
    Indeterminate 
} CheckboxState;
typedef struct Checkbox Checkbox;

NVAPI Checkbox* CheckboxCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI void CheckboxSetState(Checkbox *checkbox, CheckboxState state);
NVAPI CheckboxState CheckboxGetState(Checkbox *checkbox);
