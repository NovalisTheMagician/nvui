#include "nvui/widgets.h"

#include <tgmath.h>

#include "nvui/private/widgets.h"
#include "nvui/window.h"
#include "nvui/painter.h"
#include "nvui/font.h"

static int LabelMessage(Element *element, Message message, int di, void *dp)
{
    Label *label = (Label*)element;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        PainterSetColor(painter, COLOR_BLACK);
        PainterDrawString(painter, element->bounds, label->text, label->textBytes, element->flags & LABEL_CENTER);
    }
    else if(message == MSG_GET_WIDTH)
    {
        Font *font = WindowGetFontVariant(element->window, Serif);
        RectangleF textRect = FontMeasureString(font, Regular, label->text, label->textBytes);
        return round(textRect.r - textRect.l);
    }
    else if(message == MSG_GET_HEIGHT)
    {
        Font *font = WindowGetFontVariant(element->window, Serif);
        return round(FontGetHeight(font, Regular));
    }

    return 0;
}

NVAPI Label* LabelCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes)
{
    Label *label = (Label*)ElementCreate(sizeof *label, parent, flags, LabelMessage);
    StringCopy(&label->text, &label->textBytes, text, textBytes);
    return label;
}

NVAPI void LabelSetContent(Label *label, const char *text, ssize_t textBytes)
{
    StringCopy(&label->text, &label->textBytes, text, textBytes);
    ElementRepaint(&label->e, NULL);
}

NVAPI Element* LabelGetElement(Label *label)
{
    return &label->e;
}
