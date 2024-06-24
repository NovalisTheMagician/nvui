#include "nvui/widgets.h"

#include <stdlib.h>
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
        Color c = COLOR_BLACK;
        ElementMessage(element, MSG_LABEL_GET_COLOR, 0, &c);
        PainterSetColor(painter, c);
        PainterDrawString(painter, element->bounds, label->text, label->textBytes, element->flags & LABEL_CENTER);
    }
    else if(message == MSG_DESTROY)
    {
        free(label->text);
    }
    else if(message == MSG_GET_WIDTH)
    {
        Font *font = WindowGetFontVariant(element->window, DefaultVariant);
        float w = FontMeasureString(font, DefaultStyle, label->text, label->textBytes, 0);
        return round(w);
    }
    else if(message == MSG_GET_HEIGHT)
    {
        Font *font = WindowGetFontVariant(element->window, DefaultVariant);
        return round(FontGetHeight(font, DefaultStyle));
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
