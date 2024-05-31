#include "nvui/painter.h"

#include <glad2/gl.h>

NVAPI void PainterDrawLine(Painter *painter, int x1, int y1, int x2, int y2)
{

}

NVAPI void PainterDrawRect(Painter *painter, Rectangle rectangle)
{

}

NVAPI void PainterFillRect(Painter *painter, Rectangle rectangle)
{

}

NVAPI void PainterClear(Painter *painter)
{
    glClearColor(painter->backColor.r, painter->backColor.g, painter->backColor.b, painter->backColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}
