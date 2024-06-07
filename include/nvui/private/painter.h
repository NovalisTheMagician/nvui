#pragma once

#include "../painter.h"
#include "../color.h"
#include "../util.h"
#include "../gltypes.h"

#include "glad/gl.h"

typedef struct Painter
{
    Rectangle clip;
    int width, height;
    Color backColor, frontColor;
    StrokeStyle strokeStyle;
    FillStyle fillStyle;
    float lineWidth;

    Font *defaultFont, *font;
    FontStyle fontStyle;

    GLuint fontProgram, program;
    GLuint framebuffer, textureLoc, tintLoc, defaultTexture;
    Vertex *vertexMap;

    size_t vertIndex;
} Painter;
