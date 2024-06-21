#include "nvui/widgets.h"

#include "nvui/private/widgets.h"
#include "nvui/painter.h"

#include "nvui/color.h"

static int FlowPanelLayout(FlowPanel *panel, Rectangle bounds, bool measure)
{
    bool horizontal = panel->e.flags & FLOWPANEL_HORIZONTAL;

    int position = horizontal ? panel->border.l : panel->border.t;
    int border2 = horizontal ? panel->border.t : panel->border.l;
    int hSpace = bounds.r - bounds.l - panel->border.r - panel->border.l;
    int vSpace = bounds.b - bounds.t - panel->border.b - panel->border.t;

    int available = horizontal ? hSpace : vSpace;

    int fill = 0;
    int perFill = 0;
    int count = 0;

    for(size_t i = 0; i < panel->e.childCount; ++i)
    {
        if(panel->e.children[i]->flags & ELEMENT_DESTROY) continue;

        count++;
        if(horizontal)
        {
            if(panel->e.children[i]->flags & ELEMENT_H_FILL)
            {
                fill++;
            }
            else if(available > 0)
            {
                available -= ElementMessage(panel->e.children[i], MSG_GET_WIDTH, vSpace, NULL);
            }
        }
        else
        {
            if(panel->e.children[i]->flags & ELEMENT_V_FILL)
            {
                fill++;
            }
            else if(available > 0)
            {
                available -= ElementMessage(panel->e.children[i], MSG_GET_HEIGHT, hSpace, NULL);
            }
        }
    }

    if(count)
    {
        available -= (count - 1) * panel->gap;
    }

    if(available > 0 && fill)
    {
        perFill = available / fill;
    }

    for(size_t i = 0; i < panel->e.childCount; ++i)
    {
        if(panel->e.children[i]->flags & ELEMENT_DESTROY) continue;

        Element *child = panel->e.children[i];

        if(horizontal)
        {
            int height = child->flags & ELEMENT_V_FILL ? vSpace : ElementMessage(child, MSG_GET_HEIGHT, (child->flags & ELEMENT_H_FILL) ? perFill : 0, NULL);
            int width = child->flags & ELEMENT_H_FILL ? perFill : ElementMessage(child, MSG_GET_WIDTH, height, NULL);

            Rectangle r = (Rectangle){ position + bounds.l, position + width + bounds.l, border2 + (vSpace - height) / 2 + bounds.t, border2 + (vSpace + height) / 2 + bounds.t };
            if(!measure)
            {
                ElementMove(child, r, false);
            }

            position += width + panel->gap;
        }
        else
        {
            int width = child->flags & ELEMENT_H_FILL ? hSpace : ElementMessage(child, MSG_GET_WIDTH, (child->flags & ELEMENT_V_FILL) ? perFill : 0, NULL);
            int height = child->flags & ELEMENT_V_FILL ? perFill : ElementMessage(child, MSG_GET_HEIGHT, width, NULL);

            Rectangle r = (Rectangle){ border2 + (hSpace - width) / 2 + bounds.l, border2 + (hSpace + width) / 2 + bounds.l, position + bounds.t, position + height + bounds.t };
            if(!measure)
            {
                ElementMove(child, r, false);
            }

            position += height + panel->gap;
        }
    }

    return position - (panel->e.childCount ? panel->gap : 0) + (horizontal ? panel->border.r : panel->border.b);
}

static int FlowPanelMeasure(FlowPanel *panel)
{
    bool horizontal = panel->e.flags & FLOWPANEL_HORIZONTAL;

    int size = 0;
    for(size_t i = 0; i < panel->e.childCount; ++i)
    {
        if(panel->e.children[i]->flags & ELEMENT_DESTROY) continue;

        int childSize = ElementMessage(panel->e.children[i], horizontal ? MSG_GET_HEIGHT : MSG_GET_WIDTH, 0, NULL);

        if(childSize > size)
            size = childSize;
    }
    int border = horizontal ? (panel->border.t + panel->border.b) : (panel->border.l + panel->border.r);
    return size + border;
}

static int FlowPanelMessage(Element *element, Message message, int di, void *dp)
{
    FlowPanel *panel = (FlowPanel*)element;
    bool horizontal = element->flags & FLOWPANEL_HORIZONTAL;

    if(message == MSG_PAINT)
    {
        Painter *painter = dp;
        Color backColor = COLOR_CONTROL;
        ElementMessage(element, MSG_PANEL_GET_COLOR, 0, &backColor);
        Rectangle bounds = ElementGetBounds(element);
        PainterSetColor(painter, backColor);
        PainterFillRect(painter, bounds);
        if(element->flags & PANEL_BORDER)
        {
            if(element->flags & PANEL_BORDER_3D)
            {
                Color brighter = ColorMultiply(backColor, 2);
                Color darker = ColorMultiply(backColor, 0.5f);
                PainterDrawRectLit(painter, bounds, brighter, darker);
            }
            else
            {
                Color borderColor = COLOR_BLACK;
                ElementMessage(element, MSG_PANEL_GET_BORDER_COLOR, 0, &borderColor);
                PainterSetColor(painter, COLOR_BLACK);
                PainterDrawRect(painter, bounds);
            }
        }
    }
    else if(message == MSG_LAYOUT)
    {
        FlowPanelLayout(panel, element->bounds, false);
        ElementRepaint(element, NULL);
    }
    else if(message == MSG_GET_WIDTH)
    {
        return horizontal ? FlowPanelLayout(panel, (Rectangle){ .b = di }, true) : FlowPanelMeasure(panel);
    }
    else if(message == MSG_GET_HEIGHT)
    {
        return horizontal ? FlowPanelMeasure(panel) : FlowPanelLayout(panel, (Rectangle){ .r = di }, true);
    }

    return 0;
}

NVAPI FlowPanel* FlowPanelCreate(Element *parent, uint32_t flags)
{
    return (FlowPanel*)ElementCreate(sizeof(FlowPanel), parent, flags, FlowPanelMessage);
}

NVAPI void FlowPanelSetGap(FlowPanel *panel, int gap)
{
    panel->gap = gap;
}

NVAPI void FlowPanelSetBorder(FlowPanel *panel, Rectangle border)
{
    panel->border = border;
}
