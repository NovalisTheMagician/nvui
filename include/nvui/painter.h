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

typedef struct Painter Painter;

NVAPI void PainterReset(Painter *painter);
NVAPI void PainterSetColor(Painter *painter, Color color);
NVAPI void PainterSetLineWidth(Painter *painter, float width);
NVAPI void PainterSetFont(Painter *painter, Font *font);
NVAPI void PainterSetFontStyle(Painter *painter, FontStyle style);

NVAPI Font* PainterGetFont(Painter *painter);
NVAPI FontStyle PainterGetFontStyle(Painter *painter);

NVAPI void PainterSetClip(Painter *painter, Rectangle newClip);
NVAPI void PainterRestoreClip(Painter *painter);

NVAPI void PainterDrawLine(Painter *painter, float x1, float y1, float x2, float y2);
NVAPI void PainterDrawRect(Painter *painter, Rectangle rectangle);
NVAPI void PainterDrawCircle(Painter *painter, Rectangle rectangle);
NVAPI void PainterDrawEllipse(Painter *painter, Rectangle rectangle);
NVAPI void PainterDrawString(Painter *painter, Rectangle bounds, const char *string, size_t bytes, bool centerAlign);

NVAPI void PainterFillRect(Painter *painter, Rectangle rectangle);
NVAPI void PainterFillCircle(Painter *painter, Rectangle rectangle);
NVAPI void PainterFillEllipse(Painter *painter, Rectangle rectangle);

NVAPI void PainterClear(Painter *painter);
