#include "nvui/private/widgets.h"

#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

#include "nvui/util.h"
#include "nvui/painter.h"
#include "nvui/font.h"
#include "nvui/window.h"
#include "nvui/keys.h"

#define PADDING 3

static int TextfieldMessage(Element *element, Message message, int di, void *dp)
{
    Textfield *textfield = (Textfield*)element;
    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        Rectangle bounds = ElementGetBounds(element);
        bool focused = WindowGetFocused(element->window) == element;

        Color c = COLOR_WHITE;
        ElementMessage(element, MSG_TEXTFIELD_GET_COLOR, focused, &c);
        PainterSetColor(painter, c);
        PainterFillRect(painter, bounds);

        Color textColor = COLOR_BLACK;
        ElementMessage(element, MSG_TEXTFIELD_GET_TEXT_COLOR, focused, &textColor);
        PainterSetColor(painter, textColor);
        Rectangle textBounds = bounds;
        textBounds.l += PADDING;
        textBounds.t += PADDING;
        textBounds.r -= PADDING;
        textBounds.b -= PADDING;
        PainterDrawString(painter, textBounds, textfield->text, textfield->textBytes, false);

        if(focused)
        {
            Font *font = WindowGetFontVariant(element->window, DefaultVariant);
            float w = FontMeasureString(font, DefaultStyle, textfield->text, textfield->cursorPos, 0);
            float offset = round(w);
            PainterDrawLine(painter, textBounds.l + offset, textBounds.t, textBounds.l + offset, textBounds.b);
        }

        Color brighter = ColorFromGrayscale(0.9f); // ColorMultiply(baseColor, 2);
        Color darker = ColorFromGrayscale(0.25f); // ColorMultiply(baseColor, 0.5f);
        PainterDrawRectLit(painter, bounds, brighter, darker);
    }
    else if(message == MSG_DESTROY)
    {
        free(textfield->text);
    }
    else if(message == MSG_UPDATE)
    {
        if(WindowGetFocused(element->window) == element) // gained focus
        {
            textfield->cursorPos = textfield->textBytes;
        }
        ElementRepaint(element, NULL);
    }
    else if(message == MSG_GET_WIDTH)
    {
        return 64 + PADDING * 2;
    }
    else if(message == MSG_GET_HEIGHT)
    {
        Font *font = WindowGetFontVariant(element->window, DefaultVariant);
        float height = FontGetHeight(font, DefaultStyle);
        return round(height) + PADDING * 2;
    }
    else if(message == MSG_CHAR)
    {
        if(di == '\n')
        {
            return 0;
        }
        else if(di == '\b' && textfield->cursorPos > 0)
        {
            if(textfield->cursorPos == textfield->textBytes)
            {
                textfield->text[--textfield->cursorPos] = '\0';
                textfield->textBytes--;
            }
            else
            {
                memcpy(textfield->text + (textfield->cursorPos - 1), textfield->text + textfield->cursorPos, textfield->textBytes - textfield->cursorPos + 1);
                textfield->cursorPos--;
                textfield->textBytes--;
            }
        }
        else if(di >= ' ' && di <= 0x7f && textfield->textBytes < textfield->maxTextBytes)
        {
            if(textfield->cursorPos == textfield->textBytes)
            {
                textfield->text[textfield->cursorPos++] = di;
                textfield->textBytes++;
            }
            else
            {
                memcpy(textfield->text + textfield->cursorPos + 1, textfield->text + textfield->cursorPos, textfield->textBytes - textfield->cursorPos + 1);
                textfield->text[textfield->cursorPos++] = di;
                textfield->textBytes++;
            }
        }
        else
        {
            return 0;
        }

        ElementRepaint(element, NULL);
        ElementMessage(element, MSG_TEXTFIELD_TEXT_CHANGE, 0, NULL);
    }
    else if(message == MSG_KEY_DOWN)
    {
        if(di == KEY_LEFT && textfield->cursorPos > 0)
        {
            textfield->cursorPos--;
        }
        else if(di == KEY_RIGHT && textfield->cursorPos < textfield->textBytes)
        {
            textfield->cursorPos++;
            if(textfield->cursorPos > textfield->textBytes)
                textfield->cursorPos = textfield->textBytes;
        }
        else
        {
            return 0;
        }
        ElementRepaint(element, NULL);
    }
    return 0;
}

NVAPI Textfield* TextfieldCreate(Element *parent, uint32_t flags, size_t maxChars)
{
    Textfield *textfield = (Textfield*)ElementCreate(sizeof *textfield, parent, flags, TextfieldMessage);
    textfield->text = calloc(maxChars + 1, sizeof *textfield->text);
    textfield->maxTextBytes = maxChars;
    return textfield;
}

NVAPI void TextfieldSetText(Textfield *textfield, const char *text, ssize_t textBytes)
{
    textBytes = textBytes == -1 ? strlen(text) : textBytes;
    size_t bytes = textBytes < textfield->maxTextBytes ? textBytes : textfield->maxTextBytes;
    memcpy(textfield->text, text, bytes);
    textfield->textBytes = bytes;
    ElementRepaint((Element*)textfield, NULL);
}

NVAPI char* TextfieldGetText(Textfield *textfield, size_t *textBytes)
{
    if(textBytes) *textBytes = textfield->textBytes;
    return textfield->text;
}
