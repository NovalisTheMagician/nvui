#pragma once

#include "util.h"
#include "color.h"

#include "glad/gl.h"

typedef enum StrokeStyle
{
    STROKE_FULL,
    STROKE_DASHED,
    STROKE_DOTTED,
    STROKE_DASH_DOT,
    STROKE_DASH_DOT_DOT,
} StrokeStyle;

typedef enum FillStyle
{
    FILL_FULL,
    FILL_HATCH,
} FillStyle;

typedef struct Painter
{
    Rectangle clip;
    int width, height;
    Color backColor, frontColor;
    StrokeStyle strokeStyle;
    FillStyle fillStyle;

    GLuint framebuffer;
} Painter;

NVAPI void PainterDrawLine(Painter *painter, int x1, int y1, int x2, int y2);
NVAPI void PainterDrawRect(Painter *painter, Rectangle rectangle);
NVAPI void PainterFillRect(Painter *painter, Rectangle rectangle);
NVAPI void PainterClear(Painter *painter);
