#include "nvui/util.h"

#include <stdlib.h>
#include <string.h>

NVAPI bool RectangleValid(Rectangle a)
{
    return a.r > a.l && a.b > a.t;
}

NVAPI Rectangle RectangleIntersection(Rectangle a, Rectangle b)
{
    if(a.l < b.l) a.l = b.l;
    if(a.t < b.t) a.t = b.t;
    if(a.r > b.r) a.r = b.r;
    if(a.b > b.b) a.b = b.b;
    return a;
}

NVAPI Rectangle RectangleBounding(Rectangle a, Rectangle b)
{
    if(a.l > b.l) a.l = b.l;
    if(a.t > b.t) a.t = b.t;
    if(a.r < b.r) a.r = b.r;
    if(a.b < b.b) a.b = b.b;
    return a;
}

NVAPI bool RectangleEquals(Rectangle a, Rectangle b)
{
    return (a.r == b.r) && (a.l == b.l) && (a.t == b.t) && (a.b == b.b);
}

NVAPI bool RectangleContains(Rectangle a, int x, int y)
{
    return a.l <= x && a.r > x && a.t <= y && a.b > y;
}

NVAPI void StringCopy(char **dest, size_t *destBytes, const char *source, ptrdiff_t sourceBytes)
{
    if(sourceBytes == -1) sourceBytes = strlen(source);
    *dest = realloc(*dest, sourceBytes);
    *destBytes = sourceBytes;
    memcpy(*dest, source, sourceBytes);
}
