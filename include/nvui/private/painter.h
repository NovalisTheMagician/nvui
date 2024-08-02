#pragma once

#include "../painter.h"
#include "../color.h"
#include "../util.h"
#include "../gltypes.h"

#include "window.h"

typedef struct Painter
{
    Rectangle clip;
    int width, height;
    Color backColor;
    StrokeStyle strokeStyle;
    FillStyle fillStyle;
    float lineWidth;

    Font *defaultFont, *font;
    FontStyle fontStyle;

    GLData gldata;
    Vertex *vertexMap;

    size_t vertIndex;

    float pixelSizeW, pixelSizeH;
} Painter;
