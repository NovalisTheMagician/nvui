#pragma once

#include "nvui.h"
#include <stdlib.h>

typedef struct Menu Menu;
typedef struct MenuItem MenuItem;

NVAPI Menu* MenuCreate(void);
NVAPI void MenuDestroy(Menu *menu);
NVAPI void MenuAddItem(Menu *menu, MenuItem *item);
NVAPI void MenuRemoveAll(Menu *menu);
NVAPI void MenuRemoveByName(Menu *menu, const char *name, ssize_t nameBytes);
NVAPI void MenuRemoveByCommand(Menu *menu, int command);
NVAPI MenuItem* MenuGetItemByName(Menu *menu, const char *name, ssize_t nameBytes);
NVAPI MenuItem* MenuGetItemByCommand(Menu *menu, int command);

NVAPI MenuItem* MenuItemCreate(const char *name, ssize_t nameBytes, int command);
NVAPI void MenuItemDestroy(MenuItem *item);
NVAPI void MenuItemAddItem(MenuItem *item, MenuItem *itemAdd);
NVAPI void MenuItemSetName(MenuItem *item, const char *name, ssize_t nameBytes);
NVAPI void MenuItemSetCommand(MenuItem *item, int command);
NVAPI char* MenuItemGetName(MenuItem *item, size_t *bytes);
NVAPI int MenuItemGetCommand(MenuItem *item);
NVAPI MenuItem* MenuItemGetItemByName(MenuItem *item, const char *name, ssize_t nameBytes);
NVAPI MenuItem* MenuItemGetItemByCommand(MenuItem *item, int command);
