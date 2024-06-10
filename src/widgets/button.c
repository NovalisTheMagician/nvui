#include "nvui/widgets.h"

#include <stdlib.h>
#include <tgmath.h>

#include "nvui/window.h"
#include "nvui/private/window.h"
#include "nvui/private/widgets.h"
#include "nvui/painter.h"
#include "nvui/font.h"

static int ButtonMessage(Element *element, Message message, int di, void *dp)
{
    Button *button = (Button*)element;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;

        bool pressed = element->window->pressed == element && element->window->hovered == element;

        Color c = COLOR_WHITE;
        ElementMessage(element, MSG_BUTTON_GET_COLOR, 0, &c);
        Color c1 = pressed ? COLOR_WHITE : COLOR_BLACK, c2 = pressed ? COLOR_BLACK : c;

        //painter->backColor = ColorFromInt(c2);
        PainterSetColor(painter, c2);
        PainterFillRect(painter, element->bounds);
        //painter->backColor = ColorFromInt(c1);
        PainterSetLineWidth(painter, 1.0f);
        PainterSetColor(painter, c1);
        PainterDrawRect(painter, element->bounds);
        //painter->backColor = ColorFromInt(c1);
        PainterSetColor(painter, c1);
        PainterDrawString(painter, element->bounds, button->text, button->textBytes, true);
    }
    else if(message == MSG_DESTROY)
    {
        free(button->text);
    }
    else if(message == MSG_UPDATE)
    {
        ElementRepaint(element, NULL);
    }
    else if(message == MSG_GET_WIDTH)
    {
        Font *font = WindowGetFontVariant(element->window, Serif);
        RectangleF textRect = FontMeasureString(font, Regular, button->text, button->textBytes);
        int width = round(textRect.r - textRect.l);
        return 30 + width;
    }
    else if(message == MSG_GET_HEIGHT)
    {
        return 25;
    }

    return 0;
}

NVAPI Button* ButtonCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes)
{
    Button *button = (Button*)ElementCreate(sizeof *button, parent, flags, ButtonMessage);
    StringCopy(&button->text, &button->textBytes, text, textBytes);
    return button;
}

NVAPI Element* ButtonGetElement(Button *button)
{
    return &button->e;
}
