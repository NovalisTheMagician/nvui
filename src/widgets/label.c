#include "nvui/widgets.h"

#include "nvui/private/widgets.h"
#include "nvui/painter.h"

static int LabelMessage(Element *element, Message message, int di, void *dp)
{
    Label *label = (Label*)element;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        painter->backColor = COLOR_BLACK;
        PainterDrawString(painter, element->bounds, label->text, label->textBytes, element->flags & LABEL_CENTER);
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
