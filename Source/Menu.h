#pragma once
#ifndef Menu_H
#define Menu_H

#include <cstdint>
#include <cstring>
#include <initializer_list>]
#include <functional>
#include "NimbleDraw.h"
#include "SimpleArray.h"

void InitMenu();

class MenuItem {
public:
    virtual void onSelect() = 0;
    const char* label;
    uint8_t flags;
    const char* check;  // nullptr or pointer to UTF8 representation of a character to print in "checkmark" position.
    // Flag bits
    static const int disabled = 1;
    static const int separator = 2;
    MenuItem(const char* label_) : label(label_), flags(0), check(nullptr) {}
};

class Menu {
    SimpleArray<MenuItem*> items;
    const char* label;
    short x, y;             // Upper left corner
    uint8_t hilightRow;     // 0 = hide items, 1 = highlight none, 2+k = hilight row k
    uint16_t itemHeight;    // Height of each item (in pixels) or tab
    uint16_t itemWidth;     // Width of widest item (in pixels)
    uint16_t tabWidth;      // Width of tab
    NimbleRect tabRect;     // Rectangle bounding the tab (after most recent drawing of the menum)
    NimbleRect itemsRect;   // Rectangle bounding the items (after most recent drawing of the menum)
    void computeTabSize();
public:
    Menu() {
        std::memset(this,0,sizeof(this));
    }
    Menu(const char* label_, std::initializer_list<MenuItem*> l) : items(l), label(label_), hilightRow(0), itemHeight(0), itemWidth(0), tabWidth(0) {
        hilightRow = 0;
    }
    bool trackMouse(MouseEvent e, int x, int y);
    void draw(NimblePixMap& map, int x, int y);
};

#endif /* Menu_H */
