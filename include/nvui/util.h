#pragma once

#include "nvui.h"

#include <stdlib.h>
typedef struct Rectangle
{
    int l, r, t, b;
} Rectangle;

NVAPI bool RectangleValid(Rectangle r);
NVAPI Rectangle RectangleIntersection(Rectangle a, Rectangle b);
NVAPI Rectangle RectangleBounding(Rectangle a, Rectangle b);
NVAPI bool RectangleEquals(Rectangle a, Rectangle b);
NVAPI bool RectangleContains(Rectangle r, int x, int y);

NVAPI void StringCopy(char **dest, size_t *destBytes, const char *source, ptrdiff_t sourceBytes);

#define Min(a, b) ({ __typeof__(a) a_ = a; __typeof__(b) b_ = b; a_ < b_ ? a_ : b_; })
#define Max(a, b) ({ __typeof__(a) a_ = a; __typeof__(b) b_ = b; a_ > b_ ? a_ : b_; })
#define Clamp(x, mi, ma) ({ __typeof__(x) x_ = x; __typeof__(mi) mi_ = mi; __typeof__(ma) ma_ = ma; Min(mi_, Max(x_, ma_)); })
