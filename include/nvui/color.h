#pragma once

#include "nvui.h"

#include <cglm/struct.h>

typedef struct Color
{
    float r;
    float g;
    float b;
    float a;
} Color;

NVAPI Color ColorFromRGB(int r, int g, int b);
NVAPI Color ColorFromRGBA(int r, int g, int b, int a);
NVAPI Color ColorFromInt(int rgb);
NVAPI Color ColorFromIntAlpha(int rgba);
NVAPI Color ColorFromHSV(float h, float s, float v);
NVAPI Color ColorFromGrayscale(float scale);
NVAPI Color ColorFromGrayscaleAlpha(float scale, float alpha);

NVAPI Color ColorMultiply(Color color, float v);

NVAPI vec4s ColorToVec4(Color color);

#define COLOR_BLACK (Color){ 0, 0, 0, 1 }
#define COLOR_WHITE (Color){ 1, 1, 1, 1 }
#define COLOR_RED (Color){ 1, 0, 0, 1 }
#define COLOR_GREEN (Color){ 0, 1, 0, 1 }
#define COLOR_BLUE (Color){ 0, 0, 1, 1 }

#define COLOR_CONTROL (Color){ 0.831f, 0.815f, 0.784f, 1 }
