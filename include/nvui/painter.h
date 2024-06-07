#pragma once

#include "util.h"
#include "color.h"
#include "font.h"

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

struct Painter;
typedef struct Painter Painter;

NVAPI void PainterReset(Painter *painter);
NVAPI void PainterSetColor(Painter *painter, Color color);
NVAPI void PainterSetLineWidth(Painter *painter, float width);
NVAPI void PainterSetFont(Painter *painter, Font *font);
NVAPI void PainterSetFontStyle(Painter *painter, FontStyle style);

NVAPI void PainterDrawLine(Painter *painter, int x1, int y1, int x2, int y2);
NVAPI void PainterDrawRect(Painter *painter, Rectangle rectangle);
NVAPI void PainterFillRect(Painter *painter, Rectangle rectangle);
NVAPI void PainterDrawString(Painter *painter, Rectangle bounds, const char *string, size_t bytes, bool centerAlign);
NVAPI void PainterClear(Painter *painter);
