#pragma once

#include <stdlib.h>

typedef struct MenuItem
{
    char *name;
    size_t nameBytes;

    struct MenuItem **items;
    size_t numItems;

    int command;

    bool disabled;
} MenuItem;

typedef struct Menu 
{
    MenuItem **items;
    size_t numItems;
} Menu;
