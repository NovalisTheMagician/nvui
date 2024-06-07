#pragma once

#include "../font.h"

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
