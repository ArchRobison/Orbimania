#include "Menu.h"
#include "Host.h"

static HostFont MenuFont;

void InitMenu() {
    if( MenuFont.isOpen() )
        return;
    MenuFont.open("Roboto-Regular.ttf", 24);
}

enum {
    showNone    = 0,
    hilightNone = 1,
    hilightBase = 2
};

bool Menu::trackMouse(MouseEvent e, int x, int y) {
    if(hilightRow != showNone) {
        if(itemsRect.contains(x, y)) {
            int row = (y - itemsRect.top) / itemHeight;
            switch(e) {
                case MouseEvent::up:
                    items[row].onSelect();
                    hilightRow = showNone;
                    break;
                case MouseEvent::move:
                case MouseEvent::down:
                case MouseEvent::drag:
                    hilightRow = hilightBase + row;
                    break;
            }
            return true;
        } else {
            if(e == MouseEvent::down) {
                hilightRow = showNone;
                return tabRect.contains(x, y);
            }
        }
    }
    if(tabRect.contains(x, y)) {
        if(e == MouseEvent::down) {
            if(hilightRow == showNone) {
                hilightRow = hilightNone;
            } else {
                hilightRow = showNone;
            }
        }
        return true;
    }
    return false;
}