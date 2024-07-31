#include "nvui/menu.h"
#include "nvui/private/menu.h"

#include "nvui/util.h"
#include <string.h>

NVAPI Menu* MenuCreate(void)
{
    return calloc(1, sizeof(Menu));
}

NVAPI void MenuDestroy(Menu *menu)
{
    for(size_t i = 0; i < menu->numItems; ++i)
    {
        MenuItemDestroy(menu->items[i]);
    }
    free(menu->items);
    free(menu);
}

NVAPI void MenuAddItem(Menu *menu, MenuItem *item)
{
    menu->numItems++;
    menu->items = realloc(menu->items, menu->numItems * sizeof(MenuItem*));
    menu->items[menu->numItems-1] = item;
}

NVAPI void MenuRemoveAll(Menu *menu)
{
    for(size_t i = 0; i < menu->numItems; ++i)
    {
        MenuItemDestroy(menu->items[i]);
    }
    menu->numItems = 0;
    free(menu->items);
    menu->items = NULL;
}

NVAPI void MenuRemoveByName(Menu *menu, const char *name, ssize_t nameBytes)
{

}

NVAPI void MenuRemoveByCommand(Menu *menu, int command)
{

}

NVAPI MenuItem* MenuGetItemByName(Menu *menu, const char *name, ssize_t nameBytes)
{
    for(size_t i = 0; i < menu->numItems; ++i)
    {
        MenuItem *item = MenuItemGetItemByName(menu->items[i], name, nameBytes);
        if(item) return item;
    }
    return NULL;
}

NVAPI MenuItem* MenuGetItemByCommand(Menu *menu, int command)
{
    for(size_t i = 0; i < menu->numItems; ++i)
    {
        MenuItem *item = MenuItemGetItemByCommand(menu->items[i], command);
        if(item) return item;
    }
    return NULL;
}

NVAPI MenuItem* MenuItemCreate(const char *name, ssize_t nameBytes, int command)
{
    MenuItem *item = calloc(1, sizeof *item);

    StringCopy(&item->name, &item->nameBytes, name, nameBytes);
    item->command = command;

    return item;
}

NVAPI void MenuItemDestroy(MenuItem *item)
{
    for(size_t i = 0; i < item->numItems; ++i)
    {
        MenuItemDestroy(item->items[i]);
    }
    free(item->name);
    free(item->items);
    free(item);
}

NVAPI void MenuItemAddItem(MenuItem *item, MenuItem *itemAdd)
{
    item->numItems++;
    item->items = realloc(item->items, item->numItems * sizeof(MenuItem*));
    item->items[item->numItems-1] = itemAdd;
}

NVAPI void MenuItemSetName(MenuItem *item, const char *name, ssize_t nameBytes)
{
    StringCopy(&item->name, &item->nameBytes, name, nameBytes);
}

NVAPI void MenuItemSetCommand(MenuItem *item, int command)
{
    item->command = command;
}

NVAPI char* MenuItemGetName(MenuItem *item, size_t *bytes)
{
    if(bytes)
        *bytes = item->nameBytes;
    return item->name;
}

NVAPI int MenuItemGetCommand(MenuItem *item)
{
    return item->command;
}

NVAPI MenuItem* MenuItemGetItemByName(MenuItem *item, const char *name, ssize_t nameBytes)
{
    size_t bytes = nameBytes == -1 ? strlen(name) : nameBytes;
    if(item->nameBytes == bytes && strncasecmp(item->name, name, bytes) == 0) return item;
    for(size_t i = 0; i < item->numItems; ++i)
    {
        MenuItem *it = MenuItemGetItemByName(item->items[i], name, nameBytes);
        if(it) return it;
    }
    return NULL;
}

NVAPI MenuItem* MenuItemGetItemByCommand(MenuItem *item, int command)
{
    if(item->command == command) return item;
    for(size_t i = 0; i < item->numItems; ++i)
    {
        MenuItem *it = MenuItemGetItemByCommand(item->items[i], command);
        if(it) return it;
    }
    return NULL;
}
