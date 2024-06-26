#pragma once

#include "nvui.h"
#include "element.h"

#define ELEMENT_V_FILL (1 << 16)
#define ELEMENT_H_FILL (1 << 17)

#define BUTTON_BORDER (1 << 0)
typedef struct Button Button;

NVAPI Button* ButtonCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI void ButtonSetText(Button *button, const char *text, ssize_t textBytes);

#define LABEL_CENTER (1 << 0)
typedef struct Label Label;

NVAPI Label* LabelCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes);
NVAPI void LabelSetContent(Label *label, const char *text, ssize_t textBytes);

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

#define TEXTFIELD_BORDER (1 << 2)
typedef struct Textfield Textfield;

NVAPI Textfield* TextfieldCreate(Element *parent, uint32_t flags, size_t maxChars);
NVAPI void TextfieldSetText(Textfield *textfield, const char *text, ssize_t textBytes);
NVAPI char* TextfieldGetText(Textfield *textfield, size_t *textBytes);
