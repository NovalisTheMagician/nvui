#pragma once

#include "nvui.h"
#include "util.h"

#include <stdint.h>
#include <stb/stb_truetype.h>

#include "glad/gl.h"

typedef struct Font
{
    uint8_t *ttf;

    uint8_t *bitmap;
    int width, height;
    GLuint texture;

    stbtt_packedchar *packedchars;
} Font;

NVAPI bool FontLoad(const char *fontFile, int fontSize, Font *font);
NVAPI bool FontLoadMem(const uint8_t *data, size_t len, int fontSize, Font *font);
NVAPI RectangleF FontGetQuad(Font *font, uint32_t codepoint, float *x, float *y, float *u0, float *v0, float *u1, float *v1);
NVAPI Rectangle FontMeasureString(Font *font, const char *string, size_t bytes);
