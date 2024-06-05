#pragma once

#include "nvui.h"
#include "util.h"

#include <stdint.h>
#include <stb/stb_truetype.h>

#include "glad/gl.h"

typedef enum FontStyle
{
    Regular,
    Italic,
    Bold,
    BoldItalic,

    NumStyles
} FontStyle;

typedef struct FontStyleData
{
    uint8_t *ttf;
    uint8_t *bitmap;
    GLuint texture;
    stbtt_packedchar *packedchars;
    stbtt_fontinfo fontinfo;
} FontStyleData;

typedef struct Font
{
    int width, height;
    int size;

    bool hasStyles[NumStyles];
    FontStyleData styles[NumStyles];
} Font;

NVAPI void FontInit(Font *font, int fontSize);
NVAPI bool FontLoad(const char *fontFile, Font *font, FontStyle style);
NVAPI bool FontLoadMem(const uint8_t *data, size_t len, Font *font, FontStyle style);
NVAPI void FontFree(Font *font);
NVAPI RectangleF FontGetQuad(Font *font, FontStyle style, uint32_t codepoint, float *x, float *y, float *u0, float *v0, float *u1, float *v1);
NVAPI RectangleF FontMeasureString(Font *font, FontStyle style, const char *string, size_t bytes);
NVAPI float FontKernAdvance(Font *font, FontStyle style, uint32_t current, uint32_t next);
