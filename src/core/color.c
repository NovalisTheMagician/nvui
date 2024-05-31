#include "nvui/color.h"

#include "nvui/util.h"

NVAPI Color ColorFromRGB(int r, int g, int b)
{
    r = Clamp(r, 0, 255);
    g = Clamp(g, 0, 255);
    b = Clamp(b, 0, 255);
    return (Color){ .r = r / 255.0f, .g = g / 255.0f, .b = b / 255.0f, .a = 1 };
}

NVAPI Color ColorFromRGBA(int r, int g, int b, int a)
{
    r = Clamp(r, 0, 255);
    g = Clamp(g, 0, 255);
    b = Clamp(b, 0, 255);
    a = Clamp(a, 0, 255);
    return (Color){ .r = r / 255.0f, .g = g / 255.0f, .b = b / 255.0f, .a = a / 255.0f };
}

NVAPI Color ColorFromInt(int rgb)
{
    int r = (rgb >> 16) & 0xff;
    int g = (rgb >>  8) & 0xff;
    int b = (rgb >>  0) & 0xff;
    return (Color){ .r = r / 255.0f, .g = g / 255.0f, .b = b / 255.0f, .a = 1 };
}

NVAPI Color ColorFromIntAlpha(int rgba)
{
    int r = (rgba >> 24) & 0xff;
    int g = (rgba >> 16) & 0xff;
    int b = (rgba >>  8) & 0xff;
    int a = (rgba >>  0) & 0xff;
    return (Color){ .r = r / 255.0f, .g = g / 255.0f, .b = b / 255.0f, .a = a / 255.0f };
}

NVAPI Color ColorFromHSV(float h, float s, float v)
{
    return COLOR_BLACK;
}

NVAPI Color ColorFromGrayscale(float scale)
{
    scale = Clamp(scale, 0.0f, 1.0f);
    return (Color){ .r = scale, .g = scale, .b = scale, .a = 1 };
}

NVAPI Color ColorFromGrayscaleAlpha(float scale, float alpha)
{
    scale = Clamp(scale, 0.0f, 1.0f);
    alpha = Clamp(alpha, 0.0f, 1.0f);
    return (Color){ .r = scale, .g = scale, .b = scale, .a = alpha };
}
