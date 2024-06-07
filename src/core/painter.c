#include "nvui/private/painter.h"

#include "cglm/struct/vec2.h"
#include "nvui/private/font.h"
#include "nvui/util.h"

#include <tgmath.h>

NVAPI void PainterReset(Painter *painter)
{
    painter->backColor = COLOR_BLACK;
    painter->lineWidth = 1;
    painter->font = NULL;
    painter->fontStyle = Regular;
}

NVAPI void PainterSetColor(Painter *painter, Color color)
{
    painter->backColor = color;
}

NVAPI void PainterSetLineWidth(Painter *painter, float width)
{
    if(width > 0)
        painter->lineWidth = width;
}

NVAPI void PainterSetFont(Painter *painter, Font *font)
{
    painter->font = font;
}

NVAPI void PainterSetFontStyle(Painter *painter, FontStyle style)
{
    painter->fontStyle = style;
}

NVAPI void PainterDrawLine(Painter *painter, int x1, int y1, int x2, int y2)
{
    const size_t startVertex = painter->vertIndex;
    const vec2s dir = glms_vec2_normalize(glms_vec2_sub((vec2s){ .x = x2, .y = y2 }, (vec2s){ .x = x1, .y = y1 }));
    const vec2s leftDir = glms_vec2_scale((vec2s){ .x = dir.y, .y = -dir.x }, painter->lineWidth);
    const vec2s rightDir = glms_vec2_scale((vec2s){ .x = -dir.y, .y = dir.x }, painter->lineWidth);

    const vec2s topLeft = glms_vec2_add((vec2s){ .x = x1, .y = y1 }, rightDir);
    const vec2s topRight = glms_vec2_add((vec2s){ .x = x2, .y = y2 }, rightDir);
    const vec2s bottomLeft = glms_vec2_add((vec2s){ .x = x1, .y = y1 }, leftDir);
    const vec2s bottomRight = glms_vec2_add((vec2s){ .x = x2, .y = y2 }, leftDir);

    const vec4s white = {{ 1, 1, 1, 1 }};

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = topLeft, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = topRight, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = bottomLeft, .color = white };

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = topRight, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = bottomRight, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = bottomLeft, .color = white };

    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glDrawArrays(GL_TRIANGLES, startVertex, 6);
}

NVAPI void PainterDrawRect(Painter *painter, Rectangle rectangle)
{
    rectangle = RectangleIntersection(painter->clip, rectangle);

    PainterDrawLine(painter, rectangle.l, rectangle.t, rectangle.r, rectangle.t);
    PainterDrawLine(painter, rectangle.r, rectangle.t, rectangle.r, rectangle.b);
    PainterDrawLine(painter, rectangle.r, rectangle.b, rectangle.l, rectangle.b);
    PainterDrawLine(painter, rectangle.l, rectangle.b, rectangle.l, rectangle.t);
}

NVAPI void PainterFillRect(Painter *painter, Rectangle rectangle)
{
    const size_t startVertex = painter->vertIndex;
    rectangle = RectangleIntersection(painter->clip, rectangle);

    const vec2s topLeft = { .x = rectangle.l, .y = rectangle.t };
    const vec2s topRight = { .x = rectangle.r, .y = rectangle.t };
    const vec2s bottomLeft = { .x = rectangle.l, .y = rectangle.b };
    const vec2s bottomRight = { .x = rectangle.r, .y = rectangle.b };
    const vec4s white = {{ 1, 1, 1, 1 }};

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = topLeft, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = topRight, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = bottomLeft, .color = white };

    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = topRight, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = bottomRight, .color = white };
    painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = bottomLeft, .color = white };

    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glDrawArrays(GL_TRIANGLES, startVertex, 6);
}

NVAPI void PainterDrawString(Painter *painter, Rectangle bounds, const char *string, size_t bytes, bool centerAlign)
{
    size_t startVertex = painter->vertIndex;
    Font *font = painter->font ? painter->font : painter->defaultFont;
    FontStyle style = painter->fontStyle;

    if(!font->hasStyles[style])
    {
        style = Regular;
    }

    float x = (float)bounds.l;
    float y = (float)bounds.t;

    RectangleF rect = FontMeasureString(font, style, string, bytes);
    if(centerAlign)
    {
        x += round((bounds.r - bounds.l - rect.r - rect.l) / 2);
        y += round((bounds.b - bounds.t - rect.b - rect.t) / 2);
    }

    for(size_t i = 0; i < bytes; ++i)
    {
        uint8_t ch = string[i];

        float u0, v0, u1, v1;
        RectangleF posRect = FontGetQuad(font, style, ch, &x, &y, &u0, &v0, &u1, &v1);

        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.l, .y = posRect.t }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u0, .y = v0 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.r, .y = posRect.t }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u1, .y = v0 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.l, .y = posRect.b }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u0, .y = v1 } };

        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.r, .y = posRect.t }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u1, .y = v0 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.r, .y = posRect.b }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u1, .y = v1 } };
        painter->vertexMap[painter->vertIndex++] = (Vertex){ .position = { .x = posRect.l, .y = posRect.b }, .color = {{ 1, 1, 1, 1 }}, .texcoord = { .x = u0, .y = v1 } };

        if(i < bytes - 1)
            x += FontKernAdvance(font, painter->fontStyle, ch, string[i+1]);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(painter->fontProgram);
    glUniform1i(painter->textureLoc, 0);
    glUniform4fv(painter->tintLoc, 1, (float*)&painter->backColor);
    glBindTextureUnit(0, font->styles[style].texture);
    glDrawArrays(GL_TRIANGLES, startVertex, 6 * bytes);
    glUseProgram(painter->program);
    glDisable(GL_BLEND);
}

NVAPI void PainterClear(Painter *painter)
{
    glClearNamedFramebufferfv(painter->framebuffer, GL_COLOR, 0, (float*)&painter->backColor);
}
