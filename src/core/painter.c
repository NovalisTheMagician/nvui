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

NVAPI void PainterDrawString(Painter *painter, Rectangle bounds, const char *string, size_t bytes, bool centerAlign)
{
    size_t startVertex = painter->vertIndex;
    Font *font = painter->font ? painter->font : painter->defaultFont;
    float x = (float)bounds.l;
    float y = (float)bounds.t;
    if(centerAlign)
    {
        Rectangle rect = FontMeasureString(font, string, bytes);
        x += (bounds.r - bounds.l - rect.l - rect.r) / 2.0f;
        y += (bounds.b - bounds.t - rect.b - rect.t) / 2.0f;
    }

    for(size_t i = 0; i < bytes; ++i)
    {
        uint8_t ch = string[i];

        float u0, v0, u1, v1;
        RectangleF posRect = FontGetQuad(font, ch, &x, &y, &u0, &v0, &u1, &v1);

        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.l, .y = posRect.t }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u0, .y = v0 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.r, .y = posRect.t }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u1, .y = v0 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.l, .y = posRect.b }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u0, .y = v1 } };

        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.r, .y = posRect.t }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u1, .y = v0 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.r, .y = posRect.b }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u1, .y = v1 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.l, .y = posRect.b }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u0, .y = v1 } };
    }

    glUseProgram(painter->fontProgram);
    glUniform1i(painter->textureLoc, 0);
    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glBindTextureUnit(0, font->texture);
    glDrawArrays(GL_TRIANGLES, startVertex, 6 * bytes);
    glUseProgram(painter->program);
}

NVAPI void PainterClear(Painter *painter)
{
    glClearNamedFramebufferfv(painter->framebuffer, GL_COLOR, 0, (float*)&painter->backColor);
}

NVAPI void PainterDebug(Painter *painter)
{
    size_t startVertex = painter->vertIndex;
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)0, .y = (float)0 }, .color = {{ 1, 1, 1, 1 }}, .texcoord = {{ 0, 0 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)painter->width, .y = (float)0 }, .color = {{ 1, 1, 1, 1 }}, .texcoord = {{ 1, 0 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)0, .y = (float)painter->height }, .color = {{ 1, 1, 1, 1 }}, .texcoord = {{ 0, 1 }} };

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)painter->width, .y = (float)0 }, .color = {{ 1, 1, 1, 1 }}, .texcoord = {{ 1, 0 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)painter->width, .y = (float)painter->height }, .color = {{ 1, 1, 1, 1 }}, .texcoord = {{ 1, 1 }} };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = (float)0, .y = (float)painter->height }, .color = {{ 1, 1, 1, 1 }}, .texcoord = {{ 0, 1 }} };

    glBindTextureUnit(0, painter->defaultFont->texture);
    glDrawArrays(GL_TRIANGLES, startVertex, 6);
    glBindTextureUnit(0, painter->defaultTexture);
}
