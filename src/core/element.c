#include "nvui/element.h"

#include <stdlib.h>
#include "nvui/private/element.h"
#include "nvui/private/window.h"

const size_t ElementSize = sizeof(Element);

NVAPI Element* ElementCreate(size_t bytes, Element *parent, uint32_t flags, MessageHandler messageClass)
{
    Element *element = calloc(1, bytes);
    element->flags = flags;
    element->messageClass = messageClass;

    if(parent)
    {
        element->window = parent->window;
        element->parent = parent;
        parent->childCount++;
        parent->children = realloc(parent->children, sizeof(Element*) * parent->childCount);
        parent->children[parent->childCount - 1] = element;
    }

    return element;
}

NVAPI int ElementMessage(Element *element, Message message, int di, void *dp)
{
    if (element->messageUser)
    {
        int result = element->messageUser(element, message, di, dp);
        if(result) return result;
    }
    if(element->messageClass)
    {
        return element->messageClass(element, message, di, dp);
    }
    return 0;
}

NVAPI void ElementMove(Element *element, Rectangle bounds, bool alwaysLayout)
{
    Rectangle oldClip = element->clip;
    element->clip = RectangleIntersection(element->parent->clip, bounds);

    if(!RectangleEquals(element->bounds, bounds) || !RectangleEquals(element->clip, oldClip) || alwaysLayout)
    {
        element->bounds = bounds;
        ElementMessage(element, MSG_LAYOUT, 0, NULL);
    }
}

void InvalidateWindow(Window *window);

NVAPI void ElementRepaint(Element *element, Rectangle *region)
{
    if(!region)
    {
        region = &element->bounds;
    }

    Rectangle r = RectangleIntersection(*region, element->clip);

    if(RectangleValid(r))
    {
        if(RectangleValid(element->window->updateRegion))
        {
            element->window->updateRegion = RectangleBounding(element->window->updateRegion, r);
        }
        else
        {
            element->window->updateRegion = r;
        }

        //InvalidateWindow(element->window);
    }
}

NVAPI Element* ElementFindByPoint(Element *element, int x, int y)
{
    for(size_t i = 0; i < element->childCount; ++i)
    {
        if(RectangleContains(element->children[i]->clip, x, y))
        {
            return ElementFindByPoint(element->children[i], x, y);
        }
    }
    return element;
}

NVAPI void ElementSetUserHandler(Element *element, MessageHandler userClass)
{
    element->messageUser = userClass;
}

NVAPI Rectangle ElementGetBounds(Element *element)
{
    return element->bounds;
}
