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

    NumStyles,
    DefaultStyle = Regular
} FontStyle;

typedef struct Font Font;

NVAPI void FontInit(Font *font, int fontSize);
NVAPI bool FontLoad(const char *fontFile, Font *font, FontStyle style);
NVAPI bool FontLoadMem(const uint8_t *data, size_t len, Font *font, FontStyle style);
NVAPI void FontFree(Font *font);
NVAPI RectangleF FontGetQuad(Font *font, FontStyle style, uint32_t codepoint, float *x, float *y, float *u0, float *v0, float *u1, float *v1);
NVAPI RectangleF FontMeasureStringRect(Font *font, FontStyle style, const char *string, size_t bytes, uint32_t flags);
NVAPI float FontMeasureString(Font *font, FontStyle style, const char *string, size_t bytes, uint32_t flags);
NVAPI float FontKernAdvance(Font *font, FontStyle style, uint32_t current, uint32_t next);

NVAPI float FontGetBaseline(Font *font, FontStyle style);
NVAPI float FontGetAscender(Font *font, FontStyle style);
NVAPI float FontGetDescender(Font *font, FontStyle style);
NVAPI float FontGetLinegap(Font *font, FontStyle style);

NVAPI float FontGetHeight(Font *font, FontStyle style);
NVAPI float FontGetLineOffset(Font *font, FontStyle style);
