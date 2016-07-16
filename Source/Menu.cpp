#include "Menu.h"
#include "Host.h"
#include "NimbleDraw.h"

#pragma warning( disable : 4554 )

static HostFont MenuFont;
static HostFont CheckFont;

static int MarginWidth;
static int CheckWidth;

void InitMenu() {
    if( MenuFont.isOpen() )
        return;
    MenuFont.open("Roboto-Regular", 24);
    CheckFont.open("unicons.1.0", 24);
    CheckWidth = CheckFont.size(" ").x;
    MarginWidth = CheckWidth/4 + 1;
}

enum {
    showNone    = 0,
    hilightNone = 1,
    hilightBase = 2
};

static NimbleColor ForegroundColor(0);
static NimbleColor DisabledForegroundColor(128);
static NimbleColor BackgroundColor(255);
static NimbleColor ItemHilightColor(223, 223, 255);
static NimbleColor TabHilightColor(0, 0, 255);
static NimbleColor SeparatorColor(192);

bool Menu::trackMouse(MouseEvent e, int x, int y) {
    if(hilightRow != showNone) {
        if(itemsRect.contains(x, y)) {
            int row = (y - itemsRect.top) / itemHeight;
            switch(e) {
                case MouseEvent::up:
                    items[row]->onSelect();
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

// Compute tab size parameters
void Menu::computeTabSize() {
    int h = MenuFont.height();
    NimblePoint s = MenuFont.size(label);
    itemHeight = s.y + 1;
    tabWidth = s.x;
}

// Draw draws the menu with its upper left corner at (x,y)
void Menu::draw(NimblePixMap& map, int x, int y) {
    // Draw the tab
    if( tabWidth == 0 ) {
        // Lazily compute tabWidth and itemHeight
        computeTabSize();
    }
    NimbleColor back, fore;
    if(hilightRow != showNone) {
        back = TabHilightColor;
        fore = BackgroundColor;
    } else {
        back = BackgroundColor;
        fore = ForegroundColor;
    }
    tabRect = NimbleRect(x, y, 2*MarginWidth+tabWidth, itemHeight);
    map.draw(tabRect, back.pixel());
    MenuFont.draw( map, x+MarginWidth, y, label, fore);
    map.draw(NimbleRect(tabRect.left, tabRect.bottom, tabRect.right, tabRect.bottom + 1), SeparatorColor.pixel());

    if( hilightRow!=showNone ) {
        if( itemWidth==0 ) {
            // Lazily compute itemsWidth
            int w = tabWidth;
            for(auto& i: items) {
                int w0 = MenuFont.size(i->label).x;
                if(w0 > w) {
                    w = w0;
                }
            }
            itemWidth = CheckWidth + w + 3 + 4*MarginWidth;
        }
        int w = itemWidth;
        int h = itemHeight;
        itemsRect = NimbleRect(x, tabRect.bottom, x+w, tabRect.bottom + h*int(items.size()));

        // Draw the background
        NimbleRect r = itemsRect;
        r.left += 1;
        r.right -= 1;
        map.draw(r, BackgroundColor.pixel());

        // Draw left border
        r.left -= 1;
        r.right = r.left + 1;
        map.draw(r, SeparatorColor.pixel());

        // Draw middle border
        r.left += 1 + CheckWidth + 2*MarginWidth;
        r.right = r.left + 1;
        map.draw(r, SeparatorColor.pixel());

        // Draw right border
        r.left = itemsRect.right - 1;
        r.right = r.left + 1;
        map.draw(r, SeparatorColor.pixel());

        // Draw the items
        int checkX = x + 1 + MarginWidth;
        int labelX = x + 2 + 3*MarginWidth + CheckWidth;
        int k = 0;
        for(auto& i : items) {
            MenuItem& mi = *i;
            int yi = itemsRect.top + h*k;
            if(k == int(hilightRow-hilightBase)) {
                map.draw(NimbleRect(x, yi, x+int(itemWidth), yi+h), ItemHilightColor.pixel());
            }
            if(i == 0) {
                map.draw(NimbleRect(x, yi, itemsRect.right - 1, yi + 1), SeparatorColor.pixel());
            } else if((mi.flags&MenuItem::separator) != 0 || i == 0) {
                map.draw(NimbleRect(labelX - MarginWidth, yi, itemsRect.right - 1, yi + 1), SeparatorColor.pixel());
            }
            NimbleColor fore = (mi.flags&MenuItem::disabled) != 0 ? DisabledForegroundColor : ForegroundColor;
            if(mi.check) {
                CheckFont.draw(map, checkX, yi, mi.check, fore.pixel());
            }
            MenuFont.draw(map, labelX, yi, mi.label, fore);
            ++k;
        }
    }
}
