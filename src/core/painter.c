#include "nvui/painter.h"

NVAPI void PainterDrawLine(Painter *painter, int x1, int y1, int x2, int y2)
{
    size_t startVertex = painter->vertIndex;
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)x1, .y = (float)y1 }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)x2, .y = (float)y2 }, .color = {{ 1, 1, 1, 1 }} };

    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glDrawArrays(GL_LINES, startVertex, 2);
}

NVAPI void PainterDrawRect(Painter *painter, Rectangle rectangle)
{
    size_t startVertex = painter->vertIndex;
    rectangle = RectangleIntersection(painter->clip, rectangle);

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };

    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glDrawArrays(GL_LINES, startVertex, 8);
}

NVAPI void PainterFillRect(Painter *painter, Rectangle rectangle)
{
    size_t startVertex = painter->vertIndex;
    rectangle = RectangleIntersection(painter->clip, rectangle);

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.t }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.r, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)rectangle.l, .y = (float)rectangle.b }, .color = {{ 1, 1, 1, 1 }} };

    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glDrawArrays(GL_TRIANGLES, startVertex, 6);
}

NVAPI void PainterClear(Painter *painter)
{
    glClearNamedFramebufferfv(painter->framebuffer, GL_COLOR, 0, (float*)&painter->backColor);
}
