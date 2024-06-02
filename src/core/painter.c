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
    glClearNamedFramebufferfv(painter->framebuffer, GL_COLOR, 0, (float*)&painter->backColor);
}
