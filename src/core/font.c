#include "nvui/font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include <stdio.h>

#define FONT_WIDTH 2048
#define FONT_HEIGHT 2048

NVAPI bool FontLoad(const char *fontFile, int fontSize, Font *font)
{
    FILE *ttfFile = fopen(fontFile, "rb");
    if(!ttfFile) return false;

    fseek(ttfFile, 0, SEEK_END);
    size_t bytes = ftell(ttfFile);
    rewind(ttfFile);
    font->ttf = malloc(bytes);
    fread(font->ttf, 1, bytes, ttfFile);
    fclose(ttfFile);

    return FontLoadMem(font->ttf, bytes, fontSize, font);
}

NVAPI bool FontLoadMem(const uint8_t *data, size_t len, int fontSize, Font *font)
{
    font->bitmap = calloc(1, FONT_WIDTH * FONT_HEIGHT);
    font->packedchars = calloc(256, sizeof(stbtt_packedchar));

    if(!font->ttf)
    {
        font->ttf = malloc(len);
        memcpy(font->ttf, data, len);
    }

    font->width = FONT_WIDTH;
    font->height = FONT_HEIGHT;

    stbtt_pack_context ttctx = {};
    stbtt_PackBegin(&ttctx, font->bitmap, font->width, font->height, 0, 1, NULL);
    stbtt_PackSetOversampling(&ttctx, 8, 8);
    stbtt_PackFontRange(&ttctx, font->ttf, 0, fontSize, 0, 128, font->packedchars);
    stbtt_PackEnd(&ttctx);

    return true;
}

NVAPI RectangleF FontGetQuad(Font *font, uint32_t codepoint, float *x, float *y, float *u0, float *v0, float *u1, float *v1)
{
    stbtt_aligned_quad quad = {};
    stbtt_GetPackedQuad(font->packedchars, FONT_WIDTH, FONT_HEIGHT, codepoint, x, y, &quad, true);

    *u0 = quad.s0;
    *v0 = quad.t0;
    *u1 = quad.s1;
    *v1 = quad.t1;
    return (RectangleF){ .l = quad.x0, .r = quad.x1, .t = quad.y0, .b = quad.y1 };
}

NVAPI Rectangle FontMeasureString(Font *font, const char *string, size_t bytes)
{
    Rectangle rect = {};
    float x = 0, y = 0;
    for(size_t i = 0; i < bytes; ++i)
    {
        stbtt_aligned_quad quad = {};
        stbtt_GetPackedQuad(font->packedchars, FONT_WIDTH, FONT_HEIGHT, string[i], &x, &y, &quad, true);
        rect = RectangleBounding(rect, (Rectangle){ .l = quad.x0, .r = quad.x1, .t = quad.y0, .b = quad.y1 });
    }
    return rect;
}
