#include "nvui/private/font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include <stdio.h>

#define FONT_WIDTH 2048
#define FONT_HEIGHT 2048

NVAPI void FontInit(Font *font, int fontSize)
{
    font->size = fontSize;
    font->width = FONT_WIDTH;
    font->height = FONT_HEIGHT;
}

NVAPI bool FontLoad(const char *fontFile, Font *font, FontStyle style)
{
    FILE *ttfFile = fopen(fontFile, "rb");
    if(!ttfFile) return false;

    fseek(ttfFile, 0, SEEK_END);
    size_t bytes = ftell(ttfFile);
    rewind(ttfFile);
    font->styles[style].ttf = malloc(bytes);
    fread(font->styles[style].ttf, 1, bytes, ttfFile);
    fclose(ttfFile);

    return FontLoadMem(font->styles[style].ttf, bytes, font, style);
}

NVAPI bool FontLoadMem(const uint8_t *data, size_t len, Font *font, FontStyle style)
{
    font->styles[style].bitmap = calloc(1, FONT_WIDTH * FONT_HEIGHT);
    font->styles[style].packedchars = calloc(256, sizeof(stbtt_packedchar));

    if(!font->styles[style].ttf)
    {
        font->styles[style].ttf = malloc(len);
        memcpy(font->styles[style].ttf, data, len);
    }

    stbtt_InitFont(&font->styles[style].fontinfo, font->styles[style].ttf, 0);

    int ascend, descent, linegap;
    stbtt_GetFontVMetrics(&font->styles[style].fontinfo, &ascend, &descent, &linegap);
    float scale = stbtt_ScaleForMappingEmToPixels(&font->styles[style].fontinfo, font->size);
    font->styles[style].baseline = ascend*scale;
    font->styles[style].ascender = ascend*scale;
    font->styles[style].descender = descent*scale;
    font->styles[style].linegap = linegap*scale;
    font->styles[style].scale = scale;

    stbtt_pack_context ttctx = {};
    stbtt_PackBegin(&ttctx, font->styles[style].bitmap, font->width, font->height, 0, 1, NULL);
    stbtt_PackSetOversampling(&ttctx, 8, 8);
    stbtt_PackFontRange(&ttctx, font->styles[style].ttf, 0, STBTT_POINT_SIZE(font->size), 32, 128, font->styles[style].packedchars);
    stbtt_PackEnd(&ttctx);

    glCreateTextures(GL_TEXTURE_2D, 1, &font->styles[style].texture);
    glTextureStorage2D(font->styles[style].texture, 1, GL_R8, font->width, font->height);
    glTextureSubImage2D(font->styles[style].texture, 0, 0, 0, font->width, font->height, GL_RED, GL_UNSIGNED_BYTE, font->styles[style].bitmap);
    glTextureParameteri(font->styles[style].texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(font->styles[style].texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    font->hasStyles[style] = true;

    return true;
}

NVAPI void FontFree(Font *font)
{
    for(FontStyle style = Regular; style < NumStyles; ++style)
    {
        if(!font->hasStyles[style]) continue;
        FontStyleData *data = &font->styles[style];
        glDeleteTextures(1, &data->texture);
        free(data->bitmap);
        free(data->packedchars);
        free(data->ttf);
    }
}

NVAPI RectangleF FontGetQuad(Font *font, FontStyle style, uint32_t codepoint, float *x, float *y, float *u0, float *v0, float *u1, float *v1)
{
    FontStyleData *data = &font->styles[style];

    if(codepoint < 32 || codepoint > 127)
        codepoint = '?';

    stbtt_aligned_quad quad = {};
    stbtt_GetPackedQuad(data->packedchars, font->width, font->height, codepoint - 32, x, y, &quad, false);

    *u0 = quad.s0;
    *v0 = quad.t0;
    *u1 = quad.s1;
    *v1 = quad.t1;
    return (RectangleF){ .l = quad.x0, .r = quad.x1, .t = quad.y0, .b = quad.y1 };
}

NVAPI RectangleF FontMeasureStringRect(Font *font, FontStyle style, const char *string, size_t bytes, uint32_t flags)
{
    FontStyleData *data = &font->styles[style];

    RectangleF rect = {};
    float x = 0, y = 0;
    for(size_t i = 0; i < bytes; ++i)
    {
        uint32_t codepoint = string[i];
        if(codepoint < 32 || codepoint > 127)
            codepoint = '?';

        stbtt_aligned_quad quad = {};
        stbtt_GetPackedQuad(data->packedchars, font->width, font->height, codepoint - 32, &x, &y, &quad, false);
        if(i < bytes-1)
            x += FontKernAdvance(font, style, codepoint, string[i+1]);
        rect = RectangleFBounding(rect, (RectangleF){ .l = quad.x0, .r = quad.x1, .t = quad.y0, .b = quad.y1 });
    }
    return rect;
}

NVAPI float FontMeasureString(Font *font, FontStyle style, const char *string, size_t bytes, uint32_t flags)
{
    FontStyleData *data = &font->styles[style];

    float w = 0;
    for(size_t i = 0; i < bytes; ++i)
    {
        uint32_t codepoint = string[i];
        if(codepoint < 32 || codepoint > 127)
            codepoint = '?';

        int advancewidth, leftsidebearing;
        stbtt_GetCodepointHMetrics(&data->fontinfo, codepoint, &advancewidth, &leftsidebearing);
        w += advancewidth * data->scale;
        if(i < bytes-1)
            w += FontKernAdvance(font, style, codepoint, string[i+1]);
    }
    return w;
}

NVAPI float FontKernAdvance(Font *font, FontStyle style, uint32_t current, uint32_t next)
{
    FontStyleData *data = &font->styles[style];
    float scale = stbtt_ScaleForMappingEmToPixels(&data->fontinfo, font->size);
    return scale * stbtt_GetCodepointKernAdvance(&data->fontinfo, current, next);
}

NVAPI float FontGetBaseline(Font *font, FontStyle style)
{
    return font->styles[style].baseline;
}

NVAPI float FontGetHeight(Font *font, FontStyle style)
{
    return font->styles[style].ascender - font->styles[style].descender;
}

NVAPI float FontGetAscender(Font *font, FontStyle style)
{
    return font->styles[style].ascender;
}

NVAPI float FontGetDescender(Font *font, FontStyle style)
{
    return font->styles[style].descender;
}

NVAPI float FontGetLinegap(Font *font, FontStyle style)
{
    return font->styles[style].linegap;
}

NVAPI float FontGetLineOffset(Font *font, FontStyle style)
{
    return font->styles[style].ascender - font->styles[style].descender + font->styles[style].linegap;
}

NVAPI size_t FontGetCodepointIndexForOffset(Font *font, FontStyle style, const char *string, size_t bytes, float offset)
{
    if(offset <= 0 || !bytes)
        return 0;

    FontStyleData *data = &font->styles[style];

    float w = 0;
    for(size_t i = 0; i < bytes; ++i)
    {
        uint32_t codepoint = string[i];
        if(codepoint < 32 || codepoint > 127)
            codepoint = '?';

        int advancewidth, leftsidebearing;
        stbtt_GetCodepointHMetrics(&data->fontinfo, codepoint, &advancewidth, &leftsidebearing);
        float glyphWidth = advancewidth * data->scale;
        w += glyphWidth;
        if(i < bytes-1)
            w += FontKernAdvance(font, style, codepoint, string[i+1]);

        if(offset <= w - glyphWidth / 2)
            return i;
    }
    return bytes;
}
