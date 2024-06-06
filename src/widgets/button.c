#include "nvui/widgets.h"

#include "nvui/window.h"
#include "nvui/painter.h"

static int ButtonMessage(Element *element, Message message, int di, void *dp)
{
    Button *button = (Button*)element;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;

        bool pressed = element->window->pressed == element && element->window->hovered == element;

        uint32_t c = 0xffffff;
        ElementMessage(element, MSG_BUTTON_GET_COLOR, 0, &c);
        uint32_t c1 = pressed ? 0xffffff : 0x000000, c2 = pressed ? 0x000000 : c;

        painter->backColor = ColorFromInt(c2);
        PainterFillRect(painter, element->bounds);
        painter->backColor = ColorFromInt(c1);
        PainterDrawRect(painter, element->bounds);
        painter->backColor = ColorFromInt(c1);
        PainterDrawString(painter, element->bounds, button->text, button->textBytes, true);
    }
    else if(message == MSG_UPDATE)
    {
        ElementRepaint(element, NULL);
    }

    return 0;
}

NVAPI Button* ButtonCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes)
{
    Button *button = (Button*)ElementCreate(sizeof *button, parent, flags, ButtonMessage);
    StringCopy(&button->text, &button->textBytes, text, textBytes);
    return button;
}
