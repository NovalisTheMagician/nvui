#include "nvui/color.h"
#include "nvui/private/widgets.h"

#include "nvui/private/window.h"
#include "nvui/painter.h"
#include "nvui/widgets.h"

#include <stdlib.h>

#define BOX_SIZE 18
#define BOX_MARGIN 4

static void DrawCheckmark(Painter *painter, Rectangle bounds, bool cross)
{
    PainterSetLineWidth(painter, 2);
    if(cross)
    {
        PainterDrawLine(painter, bounds.l, bounds.t, bounds.r, bounds.b);
        PainterDrawLine(painter, bounds.l, bounds.b, bounds.r, bounds.t);
    }
    else
    {
        PainterDrawLine(painter, bounds.l, bounds.t + (bounds.b - bounds.t) / 2.0f, bounds.l + (bounds.r - bounds.l) * 0.33f, bounds.b);
        PainterDrawLine(painter, bounds.l + (bounds.r - bounds.l) * 0.33f, bounds.b, bounds.r, bounds.t);
    }
    PainterSetLineWidth(painter, 1);
}

static int CheckboxMessage(Element *element, Message message, int di, void *dp)
{
    Checkbox *checkbox = (Checkbox*)element;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        Rectangle bounds = ElementGetBounds(element);

        bool pressed = element->window->pressed == element && element->window->hovered == element;

        Rectangle boxBounds;
        boxBounds.l = bounds.l + BOX_MARGIN;
        boxBounds.t = bounds.t + BOX_MARGIN;
        boxBounds.r = boxBounds.l + BOX_SIZE;
        boxBounds.b = boxBounds.t + BOX_SIZE;

        Color backColor = pressed ? COLOR_CONTROL : COLOR_WHITE;
        ElementMessage(element, MSG_CHECKBOX_GET_COLOR, pressed, &backColor);
        PainterSetColor(painter, backColor);
        PainterFillRect(painter, boxBounds);

        Color checkColor = COLOR_BLACK;
        CheckboxState state = checkbox->state;
        ElementMessage(element, MSG_CHECKBOX_GET_CHECK_COLOR, state, &checkColor);
        PainterSetColor(painter, checkColor);
        Rectangle checkBounds = boxBounds;
        checkBounds.l += 4;
        checkBounds.t += 4;
        checkBounds.r -= 4;
        checkBounds.b -= 4;
        if(state == Checked || (state == Indeterminate && !(element->flags & CHECKBOX_TRISTATE)))
            DrawCheckmark(painter, checkBounds, element->flags & CHECKBOX_CHECK_CROSS);
        else if(state == Indeterminate)
            PainterFillRect(painter, checkBounds);

        if(element->flags & CHECKBOX_BORDER)
        {
            Color borderColor = COLOR_BLACK;
            PainterSetColor(painter, borderColor);
            PainterDrawRect(painter, boxBounds);
        }

        Rectangle borderBounds = boxBounds;
        if(element->flags & CHECKBOX_BORDER)
        {
            borderBounds.l += 1;
            borderBounds.t += 1;
            borderBounds.r -= 1;
            borderBounds.b -= 1;
        }
        Color brighter = ColorFromGrayscale(0.9f);
        Color darker = ColorFromGrayscale(0.25f);
        PainterSetColor(painter, darker);
        PainterDrawLine(painter, borderBounds.l, borderBounds.t, borderBounds.r, borderBounds.t);
        PainterDrawLine(painter, borderBounds.l, borderBounds.t, borderBounds.l, borderBounds.b);
        PainterSetColor(painter, brighter);
        PainterDrawLine(painter, borderBounds.l, borderBounds.b, borderBounds.r, borderBounds.b);
        PainterDrawLine(painter, borderBounds.r, borderBounds.b, borderBounds.r, borderBounds.t);

        Rectangle textBounds = bounds;
        textBounds.l += 2 * BOX_MARGIN + BOX_SIZE;
        Color textColor = COLOR_BLACK;
        ElementMessage(element, MSG_CHECKBOX_GET_TEXT_COLOR, 0, &textColor);
        PainterSetColor(painter, textColor);
        PainterDrawString(painter, textBounds, checkbox->text, checkbox->textBytes, true);
    }
    else if(message == MSG_DESTROY)
    {
        free(checkbox->text);
    }
    else if(message == MSG_GET_WIDTH)
    {
        Font *font = WindowGetFontVariant(element->window, DefaultVariant);
        RectangleF textRect = FontMeasureString(font, DefaultStyle, checkbox->text, checkbox->textBytes);
        int width = round(textRect.r - textRect.l);
        return 2 * BOX_MARGIN + BOX_SIZE + width;
    }
    else if(message == MSG_GET_HEIGHT)
    {
        return 2 * BOX_MARGIN + BOX_SIZE;
    }
    else if(message == MSG_UPDATE)
    {
        ElementRepaint(element, NULL);
    }
    else if(message == MSG_CLICKED)
    {
        checkbox->state = !checkbox->state;
        ElementMessage(element, MSG_CHECKBOX_STATE_CHANGE, checkbox->state, NULL);
        ElementRepaint(element, NULL);
    }

    return 0;
}

NVAPI Checkbox* CheckboxCreate(Element *parent, uint32_t flags, const char *text, ssize_t textBytes)
{
    Checkbox *checkbox = (Checkbox*)ElementCreate(sizeof *checkbox, parent, flags, CheckboxMessage);
    StringCopy(&checkbox->text, &checkbox->textBytes, text, textBytes);
    return checkbox;
}

NVAPI void CheckboxSetState(Checkbox *checkbox, CheckboxState state)
{
    checkbox->state = state;
    ElementRepaint((Element*)checkbox, NULL);
}

NVAPI CheckboxState CheckboxGetState(Checkbox *checkbox)
{
    return checkbox->state;
}
