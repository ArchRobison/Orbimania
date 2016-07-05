#pragma once
#ifndef Menu_H
#define Menu_H

#include <cstdint>
#include <cstring>
#include "NimbleDraw.h"

void InitMenu();

class MenuItem {
public:
    virtual void onSelect() = 0;
};

class Menu {
    const char* label;
    size_t nItem;
    MenuItem* items;
    short x, y;             // Upper left corner
    uint8_t hilightRow;     // 0 = hide items, 1 = highlight none, 2+k = hilight row k
    uint16_t itemHeight;    // Height of each item (in pixels) or tab
    uint16_t itemWidth;     // Width of widest item (in pixels)
    uint16_t tabWidth;      // Width of tab
    NimbleRect tabRect;     // Rectangle bounding the tab
    NimbleRect itemsRect;   // Rectangle bounding the items
public:
    Menu() {
        std::memset(this,0,sizeof(this));
    }
    bool trackMouse(MouseEvent e, int x, int y);
};



#endif /* Menu_H */
