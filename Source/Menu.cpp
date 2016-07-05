#include "Menu.h"
#include "Host.h"

static HostFont MenuFont;

void InitMenu() {
    if( MenuFont.isOpen() )
        return;
    MenuFont.open("Roboto-Regular.ttf", 24);
}
