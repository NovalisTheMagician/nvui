#include "nvui/widgets.h"

#include <stdlib.h>
#include <tgmath.h>

#include "nvui/window.h"
#include "nvui/private/window.h"
#include "nvui/private/widgets.h"
#include "nvui/painter.h"
#include "nvui/font.h"

static void DrawBorder(Painter *painter, Color baseColor, Rectangle bounds, bool raised)
{
    Color brighter = ColorFromGrayscale(0.9f); // ColorMultiply(baseColor, 2);
    Color darker = ColorFromGrayscale(0.25f); // ColorMultiply(baseColor, 0.5f);
    if(raised) // top left brighter || bottom right darker
    {
        PainterSetColor(painter, brighter);
        PainterDrawLine(painter, bounds.l, bounds.t, bounds.r, bounds.t);
        PainterDrawLine(painter, bounds.l, bounds.t, bounds.l, bounds.b);
        PainterSetColor(painter, darker);
        PainterDrawLine(painter, bounds.l, bounds.b, bounds.r, bounds.b);
        PainterDrawLine(painter, bounds.r, bounds.b, bounds.r, bounds.t);
    }
    else // top left darker || bottom right brighter
    {
        PainterSetColor(painter, darker);
        PainterDrawLine(painter, bounds.l, bounds.t, bounds.r, bounds.t);
        PainterDrawLine(painter, bounds.l, bounds.t, bounds.l, bounds.b);
        PainterSetColor(painter, brighter);
        PainterDrawLine(painter, bounds.l, bounds.b, bounds.r, bounds.b);
        PainterDrawLine(painter, bounds.r, bounds.b, bounds.r, bounds.t);
    }
}

static int ButtonMessage(Element *element, Message message, int di, void *dp)
{
    Button *button = (Button*)element;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        Rectangle bounds = ElementGetBounds(element);

        bool pressed = element->window->pressed == element && element->window->hovered == element;

        Color c = COLOR_CONTROL;
        ElementMessage(element, MSG_BUTTON_GET_COLOR, pressed, &c);

        PainterSetColor(painter, c);
        PainterFillRect(painter, bounds);

        Rectangle raiseBounds = bounds;
        if(element->flags & BUTTON_BORDER)
        {
            raiseBounds.l += 1;
            raiseBounds.t += 1;
            raiseBounds.r -= 1;
            raiseBounds.b -= 1;
        }

        DrawBorder(painter, c, raiseBounds, !pressed);

        if(element->flags & BUTTON_BORDER)
        {
            PainterSetColor(painter, COLOR_BLACK);
            PainterDrawRect(painter, bounds);
        }

        Color textColor = COLOR_BLACK;
        ElementMessage(element, MSG_BUTTON_GET_TEXT_COLOR, 0, &textColor);

        PainterSetColor(painter, textColor);
        Rectangle textBounds = bounds;
        if(pressed)
        {
            textBounds.l += 1;
            textBounds.t += 1;
            textBounds.r += 1;
            textBounds.b += 1;
        }
        PainterDrawString(painter, textBounds, button->text, button->textBytes, true);
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
        Font *font = WindowGetFontVariant(element->window, DefaultVariant);
        RectangleF textRect = FontMeasureString(font, DefaultStyle, button->text, button->textBytes);
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
