#include "nvui/private/widgets.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

#include "nvui/util.h"
#include "nvui/painter.h"
#include "nvui/font.h"
#include "nvui/window.h"
#include "nvui/keys.h"

#define PADDING 2

static void RemoveCharAt(Textfield *textfield, size_t at)
{
    memcpy(textfield->text + (at - 1), textfield->text + at, textfield->textBytes - at + 1);
    textfield->textBytes--;
}

static void InsertCharAt(Textfield *textfield, size_t at, int codepoint)
{
    memcpy(textfield->text + at + 1, textfield->text + at, textfield->textBytes - at + 1);
    textfield->text[at] = codepoint;
    textfield->textBytes++;
}

static int TextfieldMessage(Element *element, Message message, int di, void *dp)
{
    Textfield *textfield = (Textfield*)element;
    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        Rectangle bounds = ElementGetBounds(element);
        bool focused = WindowGetFocused(element->window) == element;

        Color c = COLOR_WHITE;
        Rectangle backgroundRect = bounds;
        backgroundRect.l += 1;
        backgroundRect.t += 1;
        backgroundRect.r -= 1;
        backgroundRect.b -= 1;
        ElementMessage(element, MSG_TEXTFIELD_GET_COLOR, focused, &c);
        PainterSetColor(painter, c);
        PainterFillRect(painter, backgroundRect);

        Rectangle raiseBounds = bounds;
        raiseBounds.l += 1;
        raiseBounds.t += 1;
        raiseBounds.r -= 1;
        raiseBounds.b -= 1;

        Color brighter = ColorFromGrayscale(0.9f);
        Color darker = ColorFromGrayscale(0.25f);
        PainterDrawRectLit(painter, raiseBounds, brighter, darker);

        if(element->flags & TEXTFIELD_BORDER || focused)
        {
            PainterSetColor(painter, COLOR_BLACK);
            PainterDrawRect(painter, bounds);
        }

        Color textColor = COLOR_BLACK;
        ElementMessage(element, MSG_TEXTFIELD_GET_TEXT_COLOR, focused, &textColor);
        PainterSetColor(painter, textColor);
        Rectangle textBounds = bounds;
        textBounds.l += PADDING;
        textBounds.t += PADDING;
        textBounds.r -= PADDING;
        textBounds.b -= PADDING;
        if(textfield->selEnd - textfield->selStart == 0)
            PainterDrawString(painter, textBounds, textfield->text, textfield->textBytes, false);
        else
        {
            Font *font = WindowGetFontVariant(element->window, DefaultVariant);
            float startOffset = FontMeasureString(font, DefaultStyle, textfield->text, textfield->selStart, 0);
            float endOffset = FontMeasureString(font, DefaultStyle, textfield->text + textfield->selStart, textfield->selEnd - textfield->selStart, 0);

            PainterSetColor(painter, COLOR_BLUE);
            Rectangle selectRect = textBounds;
            selectRect.l += startOffset;
            selectRect.r = selectRect.l + (endOffset - startOffset);
            PainterFillRect(painter, selectRect);

            TextColorData colorData[] =
            {
                (TextColorData){ .from = textfield->selStart, .color = COLOR_WHITE },
                (TextColorData){ .from = textfield->selEnd, .reset = true },
                TextColorDataEnd
            };
            PainterSetColor(painter, textColor);
            PainterDrawStringColored(painter, textBounds, textfield->text, textfield->textBytes, false, colorData);
        }

        if(focused)
        {
            Color cursorColor = textfield->cursorPos > textfield->selStart && textfield->cursorPos < textfield->selEnd ? COLOR_WHITE : COLOR_BLACK;
            PainterSetColor(painter, cursorColor);
            Font *font = WindowGetFontVariant(element->window, DefaultVariant);
            float w = FontMeasureString(font, DefaultStyle, textfield->text, textfield->cursorPos, 0);
            float offset = round(w);
            PainterDrawLine(painter, textBounds.l + offset, textBounds.t, textBounds.l + offset, textBounds.b);
        }        
    }
    else if(message == MSG_DESTROY)
    {
        free(textfield->text);
    }
    else if(message == MSG_UPDATE)
    {
        if(di == UPDATE_HOVER_ENTER)
        {
            WindowSetCursor(element->window, Bar);
        }
        else if(di == UPDATE_HOVER_LEAVE)
        {
            WindowSetCursor(element->window, Arrow);
        }
        else if(di == UPDATE_FOCUS_GAIN)
        {
            textfield->cursorPos = textfield->textBytes;
        }

        ElementRepaint(element, NULL);
    }
    else if(message == MSG_CLICKED)
    {
        int x, y;
        ElementGetCursorPos(element, &x, &y);

        Font *font = WindowGetFontVariant(element->window, DefaultVariant);

        // make it text relative
        x -= PADDING;
        size_t offsetIdx = FontGetCodepointIndexForOffset(font, DefaultStyle, textfield->text, textfield->textBytes, x);
        
        textfield->cursorPos = offsetIdx;
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
                RemoveCharAt(textfield, textfield->cursorPos);
                textfield->cursorPos--;
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
                InsertCharAt(textfield, textfield->cursorPos, di);
                textfield->cursorPos++;
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
        if(di == KEY_LSHIFT || di == KEY_RSHIFT)
            textfield->shiftDown = true;

        if(di == KEY_LEFT && textfield->cursorPos > 0)
        {
            textfield->cursorPos--;
            if(textfield->shiftDown)
            {
                if(textfield->cursorPos > textfield->selEnd)
                    textfield->selEnd = textfield->cursorPos;
                else
                    textfield->selStart = textfield->cursorPos;
            }
            else
                textfield->selEnd = textfield->selStart = textfield->cursorPos;
        }
        else if(di == KEY_RIGHT && textfield->cursorPos < textfield->textBytes)
        {
            textfield->cursorPos++;
            if(textfield->shiftDown)
            {
                if(textfield->cursorPos < textfield->selStart)
                    textfield->selStart = textfield->cursorPos;
                else
                    textfield->selEnd = textfield->cursorPos;
            }
            else
                textfield->selEnd = textfield->selStart = textfield->cursorPos;
        }
        else if(di == KEY_HOME && textfield->cursorPos > 0)
        {
            textfield->cursorPos = 0;
            textfield->selEnd = textfield->selStart = textfield->cursorPos;
        }
        else if(di == KEY_END && textfield->cursorPos < textfield->textBytes)
        {
            textfield->cursorPos = textfield->textBytes;
            textfield->selEnd = textfield->selStart = textfield->cursorPos;
        }
        else if(di == KEY_DELETE && textfield->cursorPos < textfield->textBytes)
        {
            RemoveCharAt(textfield, textfield->cursorPos + 1);
        }
        else
        {
            return 0;
        }
        ElementRepaint(element, NULL);
    }
    else if(message == MSG_KEY_UP)
    {
        if(di == KEY_LSHIFT || di == KEY_RSHIFT)
            textfield->shiftDown = false;
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
